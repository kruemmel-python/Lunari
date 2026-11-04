#include "leng/lunari_grammar.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>

namespace {
constexpr int kLearn = 1001;
constexpr int kStopLearning = 1002;
constexpr int kTranslateGerman = 1003;
constexpr int kTranslateLunari = 1004;
constexpr int kTitle = 2001;
constexpr int kStatus = 2002;
constexpr int kSourceLabel = 2003;
constexpr int kSource = 2004;
constexpr int kResultLabel = 2005;
constexpr int kResult = 2006;
constexpr UINT kLearningProgress = WM_APP + 1;
constexpr UINT kLearningFinished = WM_APP + 2;

std::filesystem::path project_root;
std::thread learning_thread;
std::atomic_bool learning{false};
std::atomic_bool stop_learning{false};
std::mutex status_mutex;
std::string learning_status;
std::unordered_set<std::string> session_lexicon_entries;

std::wstring utf8_to_wide(const std::string& value) {
  if (value.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(static_cast<std::size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
  return result;
}

std::string wide_to_utf8(const std::wstring& value) {
  if (value.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
  std::string result(static_cast<std::size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
  return result;
}

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

void refresh_status(HWND hwnd, const std::string& status) { set_text(hwnd, kStatus, utf8_to_wide(status)); }

std::size_t learn_text_lexemes(const std::string& text) {
  const auto target = project_root / "neu-lunari-german-v1.tsv";
  bool compatible{};
  if (std::ifstream existing(target); existing) { std::string header; compatible = static_cast<bool>(std::getline(existing, header)) && header.find("Neu-Lunari v2") != std::string::npos; }
  std::ofstream output(target, compatible ? std::ios::app : std::ios::trunc);
  if (!output) throw std::runtime_error("Der Neu-Lunari-Datensatz kann nicht zum direkten Lernen geöffnet werden.");
  if (!compatible) output << "# Neu-Lunari v2 German lexicon; directly learned entries\nGerman\tNeu-Lunari\n";
  std::string word; std::size_t learned{};
  const auto save_word = [&] {
    if (word.empty()) return;
    const auto form = leng::encode_german_neu_lunari_lexeme(word);
    if (!form.empty() && session_lexicon_entries.insert(word).second) { output << word << '\t' << form << '\n'; ++learned; }
    word.clear();
  };
  for (const unsigned char character : text) {
    if (std::isspace(character) || std::string_view{".,;:!?()[]{}\"'"}.find(static_cast<char>(character)) != std::string_view::npos) save_word();
    else word.push_back(static_cast<char>(character));
  }
  save_word(); output.flush();
  if (!output) throw std::runtime_error("Direkt gelernte Neu-Lunari-Wörter konnten nicht gespeichert werden.");
  return learned;
}

void layout(HWND hwnd) {
  RECT area{}; GetClientRect(hwnd, &area);
  const int margin = 24, gap = 16, width = std::max(300, static_cast<int>(area.right - area.left) - margin * 2);
  const int column = std::max(130, (width - gap) / 2), right_x = margin + column + gap;
  const int height = std::max(80, static_cast<int>(area.bottom - area.top) - 310);
  MoveWindow(GetDlgItem(hwnd, kTitle), margin, 22, 355, 34, TRUE);
  MoveWindow(GetDlgItem(hwnd, kLearn), 450, 22, 230, 42, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStopLearning), 700, 22, 190, 42, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStatus), margin, 82, width, 70, TRUE);
  MoveWindow(GetDlgItem(hwnd, kSourceLabel), margin, 168, column, 22, TRUE);
  MoveWindow(GetDlgItem(hwnd, kResultLabel), right_x, 168, column, 22, TRUE);
  MoveWindow(GetDlgItem(hwnd, kSource), margin, 195, column, height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kResult), right_x, 195, column, height, TRUE);
  const int button_y = 205 + height;
  MoveWindow(GetDlgItem(hwnd, kTranslateGerman), margin, button_y, column, 42, TRUE);
  MoveWindow(GetDlgItem(hwnd, kTranslateLunari), right_x, button_y, column, 42, TRUE);
}

void set_learning_controls(HWND hwnd, bool active) {
  EnableWindow(GetDlgItem(hwnd, kLearn), active ? FALSE : TRUE);
  EnableWindow(GetDlgItem(hwnd, kTranslateGerman), active ? FALSE : TRUE);
  EnableWindow(GetDlgItem(hwnd, kTranslateLunari), active ? FALSE : TRUE);
  EnableWindow(GetDlgItem(hwnd, kStopLearning), active ? TRUE : FALSE);
}

void start_lexicon_build(HWND hwnd) {
  if (learning.exchange(true)) return;
  if (learning_thread.joinable()) learning_thread.join();
  const auto source = project_root / "words.full.list.build.txt";
  if (!std::filesystem::exists(source)) { learning = false; throw std::runtime_error("Die deutsche Wortliste words.full.list.build.txt fehlt."); }
  stop_learning = false; set_learning_controls(hwnd, true);
  learning_thread = std::thread([hwnd, source] {
    try {
      const auto target = project_root / "neu-lunari-german-v1.tsv";
      std::ifstream input(source); if (!input) throw std::runtime_error("Die deutsche Wortliste kann nicht gelesen werden.");
      std::ofstream output(target, std::ios::trunc); if (!output) throw std::runtime_error("Der neue Neu-Lunari-Datensatz kann nicht geschrieben werden.");
      output << "# Neu-Lunari v2 German lexicon; generated from words.full.list.build.txt\nGerman\tNeu-Lunari\n";
      std::string word; std::size_t count{};
      while (std::getline(input, word)) {
        const auto form = leng::encode_german_neu_lunari_lexeme(word);
        if (!form.empty()) output << word << '\t' << form << '\n';
        ++count;
        if (count % 50000 == 0) { std::lock_guard lock(status_mutex); learning_status = "Neues Neu-Lunari-Lexikon wird erzeugt …\nVerarbeitete deutsche Wörter: " + std::to_string(count) + "\nAlte .leng-Daten werden nicht verwendet."; PostMessageW(hwnd, kLearningProgress, 0, 0); }
        if (stop_learning.load()) break;
      }
      output.flush();
      std::lock_guard lock(status_mutex);
      learning_status = stop_learning.load() ? "Erzeugung pausiert. Der neue Datensatz neu-lunari-german-v1.tsv enthält den bisherigen Fortschritt." : "Neues Neu-Lunari-Lexikon fertig: neu-lunari-german-v1.tsv\nVerarbeitete deutsche Wörter: " + std::to_string(count) + "\nAlte .leng-Daten wurden nicht verwendet.";
    } catch (const std::exception& error) {
      std::lock_guard lock(status_mutex); learning_status = std::string("Erzeugung des Neu-Lunari-Lexikons fehlgeschlagen: ") + error.what();
    }
    learning = false; PostMessageW(hwnd, kLearningFinished, 0, 0);
  });
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == kLearningProgress || message == kLearningFinished) {
    if (message == kLearningFinished && learning_thread.joinable()) learning_thread.join();
    if (message == kLearningFinished) set_learning_controls(hwnd, false);
    std::lock_guard lock(status_mutex); refresh_status(hwnd, learning_status); return 0;
  }
  if (message == WM_SIZE) { layout(hwnd); return 0; }
  if (message == WM_COMMAND) {
    try {
      const int id = LOWORD(wparam);
      if (id == kStopLearning) { stop_learning = true; return 0; }
      if (learning.load()) return 0;
      if (id == kLearn) { start_lexicon_build(hwnd); return 0; }
      if (id == kTranslateGerman) {
        const auto source = control_text(hwnd, kSource);
        const auto translated = leng::translate_german_lunari_text(source);
        if (!translated.ok) throw std::invalid_argument(translated.diagnostic);
        set_text(hwnd, kResult, utf8_to_wide(translated.text));
        try {
          const auto learned = learn_text_lexemes(source);
          refresh_status(hwnd, "Deutsch → Neu-Lunari v1. " + std::to_string(learned) + " neue Wortformen wurden direkt in neu-lunari-german-v1.tsv gespeichert.");
        } catch (const std::exception& error) {
          refresh_status(hwnd, std::string("Deutsch → Neu-Lunari v1 wurde übersetzt. Neue Wortformen konnten momentan nicht gespeichert werden: ") + error.what());
        }
      }
      if (id == kTranslateLunari) {
        const auto translated = leng::translate_lunari_german_text(control_text(hwnd, kResult));
        if (!translated.ok) throw std::invalid_argument(translated.diagnostic);
        set_text(hwnd, kSource, utf8_to_wide(translated.text));
        refresh_status(hwnd, "Neu-Lunari v1 → Deutsch. Es wurde kein Alt-Lunari-Wörterbuch verwendet.");
      }
    } catch (const std::exception& error) { refresh_status(hwnd, std::string("Vorgang fehlgeschlagen: ") + error.what()); }
    return 0;
  }
  if (message == WM_DESTROY) { stop_learning = true; if (learning_thread.joinable()) learning_thread.join(); PostQuitMessage(0); return 0; }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}
}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
  const wchar_t klass[] = L"NeuLunariTranslator";
  WNDCLASSW wc{}; wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = klass; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&wc)) return 1;
  HWND hwnd = CreateWindowExW(0, klass, L"LUNARI v1 — Deutsch ↔ Lunari", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 650, nullptr, nullptr, instance, nullptr);
  if (!hwnd) return 1;
  const DWORD static_style = WS_CHILD | WS_VISIBLE;
  CreateWindowW(L"STATIC", L"Neu-Lunari v1", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTitle), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Neues deutsches Lexikon erzeugen", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kLearn), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Erzeugung pausieren", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kStopLearning), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kStatus), instance, nullptr);
  CreateWindowW(L"STATIC", L"Deutsch eingeben", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kSourceLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kSource), instance, nullptr);
  CreateWindowW(L"STATIC", L"Lunari eingeben oder Ergebnis", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kResultLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kResult), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Deutsch → Lunari", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateGerman), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Lunari → Deutsch", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateLunari), instance, nullptr);
  project_root = executable_project_root();
  leng::load_neu_lunari_lexicon(project_root / "neu-lunari-german-v1.tsv");
  refresh_status(hwnd, "Bereit. Beide Übersetzungen verwenden ausschließlich Neu-Lunari v1. ‚Neues deutsches Lexikon erzeugen‘ erstellt neu-lunari-german-v1.tsv aus der Wortliste; alte .leng-Daten werden nicht verwendet.");
  set_learning_controls(hwnd, false); layout(hwnd); ShowWindow(hwnd, show);
  MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
  return 0;
}
