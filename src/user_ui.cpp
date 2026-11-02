#include "leng/engine.hpp"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <mutex>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace {
constexpr int kLearn = 1001;
constexpr int kToggleUiLanguage = 1002;
constexpr int kTranslateToLanguage = 1003;
constexpr int kTranslateBack = 1004;
constexpr int kDictionarySearch = 1005;
constexpr int kLearnWikidata = 1006;
constexpr int kStopLearning = 1007;
constexpr int kTranslateMeaning = 1008;
constexpr int kTitle = 2001;
constexpr int kStatus = 2002;
constexpr int kSourceLabel = 2003;
constexpr int kSource = 2004;
constexpr int kResultLabel = 2005;
constexpr int kResult = 2006;
constexpr int kDictionaryLabel = 2007;
constexpr int kDictionaryQuery = 2008;
constexpr int kDictionaryResult = 2009;

std::unique_ptr<leng::ResearchEngine> engine;
std::filesystem::path state_path;
std::filesystem::path project_root;
bool german_ui = true;
bool word_list_only_state = false;
std::thread local_learning_thread;
std::atomic_bool local_learning{false};
std::atomic_bool stop_local_learning{false};
std::mutex learning_status_mutex;
std::string learning_status;
constexpr UINT kLearningProgress = WM_APP + 1;
constexpr UINT kLearningFinished = WM_APP + 2;

std::wstring utf8_to_wide(const std::string& value) {
  if (value.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
  return result;
}

std::string wide_to_utf8(const std::wstring& value) {
  if (value.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
  return result;
}

const wchar_t* text(const wchar_t* de, const wchar_t* en) { return german_ui ? de : en; }

void set_text(HWND hwnd, int id, const std::wstring& value) { SetWindowTextW(GetDlgItem(hwnd, id), value.c_str()); }
std::string control_text(HWND hwnd, int id) {
  const int length = GetWindowTextLengthW(GetDlgItem(hwnd, id));
  std::wstring value(static_cast<std::size_t>(length) + 1, L'\0');
  GetWindowTextW(GetDlgItem(hwnd, id), value.data(), length + 1);
  value.resize(static_cast<std::size_t>(length));
  return wide_to_utf8(value);
}

std::filesystem::path executable_project_root() {
  wchar_t buffer[MAX_PATH]{};
  GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  return std::filesystem::path(buffer).parent_path().parent_path();
}

std::filesystem::path preferred_state(const std::filesystem::path& root) {
  for (const auto& file : {"german-wordlist-only.leng", "wikidata-100k.leng", "wikidata-milestone-10000.leng", "wikidata-milestone-1000.leng", "research-demo.leng"}) {
    const auto candidate = root / file;
    if (std::filesystem::exists(candidate)) return candidate;
  }
  throw std::runtime_error("no bundled .leng research state found");
}

std::string extract_block(const std::string& report, const std::string& header, const std::string& footer) {
  const std::size_t start = report.find(header);
  if (start == std::string::npos) return report;
  const std::size_t value_start = start + header.size();
  const std::size_t end = report.find(footer, value_start);
  return report.substr(value_start, end == std::string::npos ? std::string::npos : end - value_start);
}

void refresh_labels(HWND hwnd) {
  SetWindowTextW(hwnd, text(L"LINGUISTISCHE EMERGENZ — Lernen & Übersetzen", L"LINGUISTIC EMERGENCE — Learn & Translate"));
  set_text(hwnd, kTitle, text(L"Deine entstehende Sprache", L"Your emerging language"));
  set_text(hwnd, kLearn, text(L"Deutsche Wörter lernen", L"Learn German words"));
  set_text(hwnd, kLearnWikidata, text(L"Wikidata lernen", L"Learn Wikidata"));
  set_text(hwnd, kStopLearning, text(L"Lernen pausieren", L"Pause learning"));
  set_text(hwnd, kToggleUiLanguage, german_ui ? L"English" : L"Deutsch");
  set_text(hwnd, kSourceLabel, text(L"Deutsch oder Englisch eingeben", L"Enter German or English"));
  set_text(hwnd, kResultLabel, text(L"Lunari eingeben oder Ergebnis", L"Enter Lunari or result"));
  set_text(hwnd, kTranslateToLanguage, text(L"Deutsch/Englisch → Lunari", L"German/English → Lunari"));
  set_text(hwnd, kTranslateMeaning, text(L"Kernbedeutung → Lunari", L"Core meaning → Lunari"));
  set_text(hwnd, kTranslateBack, text(L"Lunari → Deutsch", L"Lunari → English"));
  set_text(hwnd, kDictionaryLabel, text(L"Wörterbuch durchsuchen", L"Search dictionary"));
  set_text(hwnd, kDictionarySearch, text(L"Suchen", L"Search"));
}

void refresh_status(HWND hwnd, const std::string& extra = {}) {
  const std::string loaded = german_ui ? "Geladener Wissensstand: " : "Loaded knowledge state: ";
  std::string content = loaded + state_path.filename().string() + "\n" + engine->lexicon_status();
  if (!extra.empty()) content = extra + "\n\n" + content;
  set_text(hwnd, kStatus, utf8_to_wide(content));
}

void layout(HWND hwnd) {
  RECT client{}; GetClientRect(hwnd, &client);
  const int margin = 18, gap = 12, button_height = 34;
  const int client_width = static_cast<int>(client.right - client.left);
  const int client_height = static_cast<int>(client.bottom - client.top);
  const int width = std::max(0, client_width - margin * 2);
  const int column = std::max(220, (width - gap) / 2);
  const int right_x = margin + column + gap;
  const int top = margin;
  MoveWindow(GetDlgItem(hwnd, kTitle), margin, top, std::max(170, width - 665), 34, TRUE);
  MoveWindow(GetDlgItem(hwnd, kLearn), std::max(margin, client_width - 625), top, 155, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kLearnWikidata), std::max(margin, client_width - 460), top, 145, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStopLearning), std::max(margin, client_width - 305), top, 150, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kToggleUiLanguage), std::max(margin, client_width - 145), top, 127, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStatus), margin, top + 45, width, 88, TRUE);
  const int editor_top = top + 147;
  MoveWindow(GetDlgItem(hwnd, kSourceLabel), margin, editor_top, column, 22, TRUE);
  MoveWindow(GetDlgItem(hwnd, kResultLabel), right_x, editor_top, std::max(0, client_width - right_x - margin), 22, TRUE);
  const int editor_height = std::max(105, (client_height - editor_top - 235) / 2);
  MoveWindow(GetDlgItem(hwnd, kSource), margin, editor_top + 25, column, editor_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kResult), right_x, editor_top + 25, std::max(0, client_width - right_x - margin), editor_height, TRUE);
  const int action_y = editor_top + 25 + editor_height + 10;
  const int semantic_width = std::max(120, column / 2 - gap / 2);
  MoveWindow(GetDlgItem(hwnd, kTranslateToLanguage), margin, action_y, semantic_width, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kTranslateMeaning), margin + semantic_width + gap, action_y, std::max(100, column - semantic_width - gap), button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kTranslateBack), right_x, action_y, std::max(0, client_width - right_x - margin), button_height, TRUE);
  const int dictionary_y = action_y + button_height + 18;
  MoveWindow(GetDlgItem(hwnd, kDictionaryLabel), margin, dictionary_y, 240, 22, TRUE);
  MoveWindow(GetDlgItem(hwnd, kDictionaryQuery), margin, dictionary_y + 25, std::max(160, width - 130), 28, TRUE);
  MoveWindow(GetDlgItem(hwnd, kDictionarySearch), std::max(margin, client_width - 120), dictionary_y + 25, 102, 28, TRUE);
  MoveWindow(GetDlgItem(hwnd, kDictionaryResult), margin, dictionary_y + 63, width, std::max(90, client_height - dictionary_y - 80), TRUE);
}

void initialize(HWND hwnd) {
  project_root = executable_project_root();
  state_path = preferred_state(project_root);
  word_list_only_state = state_path.filename() == "german-wordlist-only.leng";
  engine = std::make_unique<leng::ResearchEngine>();
  engine->load(state_path);
  refresh_labels(hwnd);
  ShowWindow(GetDlgItem(hwnd, kLearnWikidata), word_list_only_state ? SW_HIDE : SW_SHOW);
  refresh_status(hwnd, german_ui ? "Bereit. Übersetze eine Bedeutung oder erweitere den Wissensstand mit einem sicheren Lernschritt." : "Ready. Translate a meaning or extend the knowledge state with one safe learning step.");
}

void set_learning_controls(HWND hwnd, bool active) {
  for (const int id : {kLearn, kLearnWikidata, kToggleUiLanguage, kTranslateToLanguage, kTranslateMeaning, kTranslateBack, kDictionarySearch})
    EnableWindow(GetDlgItem(hwnd, id), (active || (word_list_only_state && id == kLearnWikidata)) ? FALSE : TRUE);
  EnableWindow(GetDlgItem(hwnd, kStopLearning), active ? TRUE : FALSE);
}

void show_learning_progress(HWND hwnd) {
  std::lock_guard lock(learning_status_mutex);
  set_text(hwnd, kStatus, utf8_to_wide(learning_status));
}

void start_local_learning(HWND hwnd) {
  if (local_learning.exchange(true)) return;
  if (local_learning_thread.joinable()) local_learning_thread.join();
  stop_local_learning = false;
  set_learning_controls(hwnd, true);
  const auto source = project_root / "words.full.list.build.txt";
  if (!std::filesystem::exists(source)) { local_learning = false; set_learning_controls(hwnd, false); throw std::runtime_error("user-provided German word list is not available"); }
  {
    std::lock_guard lock(learning_status_mutex);
    learning_status = german_ui ? "Lokale Wortliste wird im Hintergrund gelernt …" : "Learning local word list in the background …";
  }
  show_learning_progress(hwnd);
  local_learning_thread = std::thread([hwnd, source] {
    try {
      std::size_t total_created{};
      for (;;) {
        const auto progress = engine->import_german_word_list_batch(source, 50000);
        engine->save(state_path);
        total_created += progress.concepts_created;
        {
          std::lock_guard lock(learning_status_mutex);
          std::ostringstream out;
          out << (german_ui ? "Lokale Wortliste lernt im Hintergrund\n" : "Local word list learning in background\n")
              << (german_ui ? "Verarbeitete Zeilen: " : "Processed lines: ") << progress.source_line
              << "\n" << (german_ui ? "Neue Wortformen in diesem Lauf: " : "New word forms this run: ") << total_created
              << "\n" << (german_ui ? "Checkpoint wurde gespeichert. " : "Checkpoint saved. ")
              << (progress.complete ? (german_ui ? "Wortliste vollständig verarbeitet." : "Word list fully processed.")
                                    : (german_ui ? "Lernen läuft weiter; Pausieren speichert am nächsten Checkpoint." : "Learning continues; Pause saves at the next checkpoint."));
          learning_status = out.str();
        }
        PostMessageW(hwnd, kLearningProgress, 0, 0);
        if (progress.complete || stop_local_learning.load()) break;
      }
    } catch (const std::exception& error) {
      std::lock_guard lock(learning_status_mutex);
      learning_status = std::string(german_ui ? "Lokales Lernen angehalten: " : "Local learning stopped: ") + error.what();
    }
    local_learning = false;
    PostMessageW(hwnd, kLearningFinished, 0, 0);
  });
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == kLearningProgress) { show_learning_progress(hwnd); return 0; }
  if (message == kLearningFinished) {
    if (local_learning_thread.joinable()) local_learning_thread.join();
    set_learning_controls(hwnd, false); show_learning_progress(hwnd); return 0;
  }
  if (message == WM_SIZE) { layout(hwnd); return 0; }
  if (message == WM_COMMAND) {
    const int id = LOWORD(wparam);
    try {
      if (id == kStopLearning) { stop_local_learning = true; return 0; }
      if (local_learning.load()) return 0;
      if (id == kToggleUiLanguage) { german_ui = !german_ui; refresh_labels(hwnd); refresh_status(hwnd); return 0; }
      if (id == kTranslateToLanguage) {
        const std::string report = engine->translate_text(control_text(hwnd, kSource), "constructed");
        set_text(hwnd, kResult, utf8_to_wide(extract_block(report, "Lunari text:\n", "\n-- end Lunari text --")));
        if (report.find("newly learned tokens=0") == std::string::npos) engine->save(state_path);
      }
      if (id == kTranslateMeaning) {
        try {
          const std::string report = engine->translate_meaning(control_text(hwnd, kSource), "constructed");
          set_text(hwnd, kResult, utf8_to_wide(extract_block(report, "Lunari frame:\n", "\n-- end Lunari frame --")));
          refresh_status(hwnd, german_ui
              ? "Kernbedeutung als unabhängiger Lunari-Bedeutungsrahmen kodiert."
              : "Core meaning encoded as an independent Lunari meaning frame.");
        } catch (const std::invalid_argument&) {
          const std::string report = engine->translate_text(control_text(hwnd, kSource), "constructed");
          set_text(hwnd, kResult, utf8_to_wide(extract_block(report, "Lunari text:\n", "\n-- end Lunari text --")));
          if (report.find("newly learned tokens=0") == std::string::npos) engine->save(state_path);
          refresh_status(hwnd, german_ui
              ? "Keine geprüfte Kernbedeutung erkannt. Der Text wurde stattdessen sicher im reversiblen Lunari-Textkanal kodiert. Für den Bedeutungsrahmen z. B. eingeben: Der Mensch bewegt den Stein nicht gestern nahe."
              : "No reviewed core meaning recognized. The text was safely encoded with the reversible Lunari text channel instead. For a meaning frame, try: The human moves the stone not past near.");
        }
      }
      if (id == kTranslateBack) {
        const std::string target = german_ui ? "de" : "en";
        try {
          const std::string report = engine->translate_meaning(control_text(hwnd, kResult), target);
          set_text(hwnd, kSource, utf8_to_wide(report));
        } catch (const std::invalid_argument&) {
          const std::string report = engine->translate_text(control_text(hwnd, kResult), target);
          set_text(hwnd, kSource, utf8_to_wide(extract_block(report, german_ui ? "Deutsch text:\n" : "English text:\n", "\n-- end translated text --")));
        }
      }
      if (id == kDictionarySearch) set_text(hwnd, kDictionaryResult, utf8_to_wide(engine->dictionary_page(control_text(hwnd, kDictionaryQuery), 0)));
      if (id == kLearn) {
        start_local_learning(hwnd);
      }
      if (id == kLearnWikidata) {
        const auto source = project_root / "wikidata-100k.tsv";
        if (!std::filesystem::exists(source)) throw std::runtime_error("controlled Wikidata source is not available");
        const auto intake = engine->import_concepts_batch(source, 1000);
        const auto validated = engine->validate_concepts(1000);
        engine->save(state_path);
        std::ostringstream result;
        result << (german_ui ? "Lernschritt gespeichert: " : "Learning step saved: ")
               << intake.concepts_created << (german_ui ? " neue Konzepte, " : " new concepts, ")
               << intake.concepts_enriched << (german_ui ? " ergänzt, " : " enriched, ")
               << validated << (german_ui ? " validiert." : " validated.");
        refresh_status(hwnd, result.str());
      }
    } catch (const std::exception& error) {
      const std::string prefix = german_ui ? "Vorgang fehlgeschlagen: " : "Operation failed: ";
      refresh_status(hwnd, prefix + error.what());
    }
    return 0;
  }
  if (message == WM_DESTROY) { stop_local_learning = true; if (local_learning_thread.joinable()) local_learning_thread.join(); PostQuitMessage(0); return 0; }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}
}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
  const wchar_t klass[] = L"LinguistischeEmergenzUser";
  WNDCLASSW wc{}; wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = klass;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&wc)) return 1;
  HWND hwnd = CreateWindowExW(0, klass, L"LINGUISTISCHE EMERGENZ — Lernen & Übersetzen",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 740,
                              nullptr, nullptr, instance, nullptr);
  if (!hwnd) return 1;
  const DWORD static_style = WS_CHILD | WS_VISIBLE;
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTitle), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kLearn), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kLearnWikidata), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kStopLearning), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kToggleUiLanguage), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kStatus), instance, nullptr);
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kSourceLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kSource), instance, nullptr);
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kResultLabel), instance, nullptr);
  // The Lunari pane is intentionally editable: it is both the result of a
  // forward translation and the input for a Lunari-to-natural-language one.
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kResult), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateToLanguage), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateMeaning), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateBack), instance, nullptr);
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionaryLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionaryQuery), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionarySearch), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionaryResult), instance, nullptr);
  initialize(hwnd);
  set_learning_controls(hwnd, false);
  layout(hwnd);
  ShowWindow(hwnd, show);
  MSG msg{}; while (GetMessageW(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  return 0;
}
