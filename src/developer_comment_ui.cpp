#include "leng/engine.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
namespace fs = std::filesystem;
constexpr int kFolder = 1001, kAnalyze = 1002, kTranslate = 1003;
constexpr int kPath = 2001, kStatus = 2002, kLog = 2003;

std::unique_ptr<leng::ResearchEngine> engine;
fs::path state_path;
std::atomic_bool state_ready{false};
std::thread state_loader;
constexpr UINT kStateLoaded = WM_APP + 1;

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
std::string control_text(HWND hwnd, int id) {
  const int length = GetWindowTextLengthW(GetDlgItem(hwnd, id));
  std::wstring value(static_cast<std::size_t>(length) + 1, L'\0');
  GetWindowTextW(GetDlgItem(hwnd, id), value.data(), length + 1);
  value.resize(static_cast<std::size_t>(length));
  return wide_to_utf8(value);
}
void set_text(HWND hwnd, int id, const std::string& text) {
  std::string windows_text;
  windows_text.reserve(text.size() + 16);
  for (const char character : text) {
    if (character == '\n' && (windows_text.empty() || windows_text.back() != '\r')) windows_text += '\r';
    windows_text += character;
  }
  SetWindowTextW(GetDlgItem(hwnd, id), utf8_to_wide(windows_text).c_str());
}
fs::path executable_project_root() {
  wchar_t buffer[MAX_PATH]{};
  GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  return fs::path(buffer).parent_path().parent_path();
}
fs::path preferred_state(const fs::path& root) {
  const fs::path state = root / "german-wordlist-only.leng";
  if (!fs::exists(state)) throw std::runtime_error("german-wordlist-only.leng was not found beside the release");
  return state;
}
std::string extract_block(const std::string& report) {
  constexpr const char* header = "Lunari text:\n";
  constexpr const char* footer = "\n-- end Lunari text --";
  const std::size_t start = report.find(header);
  if (start == std::string::npos) return report;
  const std::size_t body = start + std::char_traits<char>::length(header);
  const std::size_t end = report.find(footer, body);
  return report.substr(body, end == std::string::npos ? std::string::npos : end - body);
}
std::string trim_copy(const std::string& value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}
bool has_text(const std::string& value) {
  return std::any_of(value.begin(), value.end(), [](unsigned char c) { return std::isalnum(c) != 0 || c >= 128; });
}
bool starts_lunari(const std::string& value) {
  const std::string trimmed = trim_copy(value);
  return trimmed.size() >= 7 && (trimmed.compare(0, 7, "Lunari:") == 0 || trimmed.compare(0, 7, "lunari:") == 0);
}
std::string translate_comment(const std::string& value, std::size_t& translated) {
  if (!has_text(value) || starts_lunari(value)) return value;
  const std::size_t first = value.find_first_not_of(" \t\r\n");
  const std::size_t last = value.find_last_not_of(" \t\r\n");
  if (first == std::string::npos) return value;
  const std::string source = value.substr(first, last - first + 1);
  const std::string lunari = extract_block(engine->translate_text(source, "constructed"));
  ++translated;
  return value.substr(0, first) + "Lunari: " + lunari + value.substr(last + 1);
}

struct TranslationResult { std::string text; std::size_t comments{}; };

// A small lexical scanner: comment delimiters are considered only in normal code,
// never while inside ordinary quoted strings.  Already translated comments stay intact.
TranslationResult translate_comments(const std::string& input, bool hash_comments) {
  enum class Mode { normal, block, single, double_quote, backtick };
  Mode mode = Mode::normal;
  TranslationResult result;
  std::string pending;
  auto append_translated = [&]() { result.text += translate_comment(pending, result.comments); pending.clear(); };
  for (std::size_t i = 0; i < input.size();) {
    const char current = input[i];
    if (mode == Mode::block) {
      if (current == '*' && i + 1 < input.size() && input[i + 1] == '/') {
        append_translated(); result.text += "*/"; i += 2; mode = Mode::normal;
      } else { pending += current; ++i; }
      continue;
    }
    if (mode != Mode::normal) {
      result.text += current;
      if (current == '\\' && i + 1 < input.size()) { result.text += input[i + 1]; i += 2; continue; }
      if ((mode == Mode::single && current == '\'') || (mode == Mode::double_quote && current == '"') || (mode == Mode::backtick && current == '`')) mode = Mode::normal;
      if (current == '\n' || current == '\r') mode = Mode::normal;
      ++i; continue;
    }
    // Raw strings are deliberately left untouched as a whole line: their contents may resemble comments.
    if (current == 'R' && i + 1 < input.size() && input[i + 1] == '"') {
      const std::size_t line_end = input.find('\n', i);
      const std::size_t end = line_end == std::string::npos ? input.size() : line_end + 1;
      result.text.append(input, i, end - i); i = end; continue;
    }
    if (current == '/' && i + 1 < input.size() && input[i + 1] == '/') {
      const std::size_t end = input.find('\n', i + 2);
      result.text += "//";
      const std::size_t body_end = end == std::string::npos ? input.size() : end;
      result.text += translate_comment(input.substr(i + 2, body_end - i - 2), result.comments);
      if (end != std::string::npos) result.text += '\n';
      i = end == std::string::npos ? input.size() : end + 1; continue;
    }
    if (current == '/' && i + 1 < input.size() && input[i + 1] == '*') { result.text += "/*"; i += 2; mode = Mode::block; continue; }
    if (hash_comments && current == '#') {
      const std::size_t end = input.find('\n', i + 1);
      result.text += '#'; const std::size_t body_end = end == std::string::npos ? input.size() : end;
      result.text += translate_comment(input.substr(i + 1, body_end - i - 1), result.comments);
      if (end != std::string::npos) result.text += '\n';
      i = end == std::string::npos ? input.size() : end + 1; continue;
    }
    result.text += current;
    if (current == '\'') mode = Mode::single;
    else if (current == '"') mode = Mode::double_quote;
    else if (current == '`') mode = Mode::backtick;
    ++i;
  }
  if (mode == Mode::block) append_translated();
  return result;
}

const std::set<std::string> kExtensions = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".java", ".kt", ".kts", ".cs", ".js", ".jsx", ".ts", ".tsx", ".go", ".rs", ".swift", ".php", ".m", ".mm", ".py", ".ps1", ".psm1", ".sh", ".bash", ".zsh", ".rb", ".pl", ".r", ".lua", ".sql", ".html", ".htm", ".css", ".scss", ".sass", ".xml", ".yml", ".yaml", ".toml", ".ini", ".cfg", ".cmake", ".gradle", ".properties", ".bat", ".cmd"};
bool is_hash_file(const fs::path& file) {
  const std::string ext = file.extension().string();
  return ext == ".py" || ext == ".ps1" || ext == ".psm1" || ext == ".sh" || ext == ".bash" || ext == ".zsh" || ext == ".rb" || ext == ".pl" || ext == ".r" || ext == ".yaml" || ext == ".yml" || ext == ".toml" || ext == ".ini" || ext == ".cfg";
}
bool skip_directory(const fs::path& path) {
  const std::string name = path.filename().string();
  return name == ".git" || name == ".lunari-comment-backups" || name == "node_modules" || name == ".vs" || name == "build" || name == "release";
}
std::string read_file(const fs::path& file) { std::ifstream stream(file, std::ios::binary); return {std::istreambuf_iterator<char>(stream), {}}; }
void write_file(const fs::path& file, const std::string& text) { std::ofstream stream(file, std::ios::binary | std::ios::trunc); if (!stream) throw std::runtime_error("cannot write " + file.string()); stream.write(text.data(), static_cast<std::streamsize>(text.size())); }
std::string timestamp() {
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm local{}; localtime_s(&local, &now); char value[32]{}; std::strftime(value, sizeof(value), "%Y%m%d-%H%M%S", &local); return value;
}
struct Change { fs::path file; std::string original; std::string translated; std::size_t comments{}; };
struct CommentPreview { std::size_t line{}; std::string text; };

// This is deliberately separate from translation: analysis must be quick and
// must not learn words or mutate the Lunari state.
std::vector<CommentPreview> find_comments(const std::string& input, bool hash_comments) {
  enum class Mode { normal, block, single, double_quote, backtick };
  Mode mode = Mode::normal;
  std::vector<CommentPreview> found;
  std::string pending;
  std::size_t line = 1, block_line = 1;
  auto record = [&](std::size_t at_line, const std::string& text) {
    const std::string clean = trim_copy(text);
    if (has_text(clean)) found.push_back({at_line, clean});
  };
  for (std::size_t i = 0; i < input.size();) {
    const char current = input[i];
    if (mode == Mode::block) {
      if (current == '*' && i + 1 < input.size() && input[i + 1] == '/') { record(block_line, pending); pending.clear(); i += 2; mode = Mode::normal; continue; }
      pending += current; if (current == '\n') ++line; ++i; continue;
    }
    if (mode != Mode::normal) {
      if (current == '\\' && i + 1 < input.size()) { if (input[i + 1] == '\n') ++line; i += 2; continue; }
      if ((mode == Mode::single && current == '\'') || (mode == Mode::double_quote && current == '"') || (mode == Mode::backtick && current == '`')) mode = Mode::normal;
      if (current == '\n') { ++line; mode = Mode::normal; }
      ++i; continue;
    }
    if (current == 'R' && i + 1 < input.size() && input[i + 1] == '"') {
      const std::size_t end = input.find('\n', i); const std::size_t until = end == std::string::npos ? input.size() : end + 1;
      if (end != std::string::npos) ++line;
      i = until;
      continue;
    }
    if (current == '/' && i + 1 < input.size() && input[i + 1] == '/') {
      const std::size_t end = input.find('\n', i + 2); const std::size_t body_end = end == std::string::npos ? input.size() : end;
      record(line, input.substr(i + 2, body_end - i - 2)); if (end != std::string::npos) ++line;
      i = end == std::string::npos ? input.size() : end + 1; continue;
    }
    if (current == '/' && i + 1 < input.size() && input[i + 1] == '*') { pending.clear(); block_line = line; i += 2; mode = Mode::block; continue; }
    if (hash_comments && current == '#') {
      const std::size_t end = input.find('\n', i + 1); const std::size_t body_end = end == std::string::npos ? input.size() : end;
      record(line, input.substr(i + 1, body_end - i - 1)); if (end != std::string::npos) ++line;
      i = end == std::string::npos ? input.size() : end + 1; continue;
    }
    if (current == '\'') mode = Mode::single;
    else if (current == '"') mode = Mode::double_quote;
    else if (current == '`') mode = Mode::backtick;
    if (current == '\n') ++line;
    ++i;
  }
  if (mode == Mode::block) record(block_line, pending);
  return found;
}

std::string analysis_report(const fs::path& root) {
  if (!fs::is_directory(root)) throw std::runtime_error("choose an existing project folder");
  std::size_t eligible = 0, comments = 0; std::vector<std::string> notes; std::ostringstream out;
  for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied), end; it != end; ++it) {
    if (it->is_directory() && skip_directory(it->path())) { it.disable_recursion_pending(); continue; }
    if (!it->is_regular_file() || kExtensions.count(it->path().extension().string()) == 0) continue;
    ++eligible;
    const fs::path relative = fs::relative(it->path(), root);
    if (it->file_size() > 5 * 1024 * 1024) { notes.push_back("Übersprungen (>5 MB): " + relative.string()); continue; }
    const std::string content = read_file(it->path());
    if (content.find('\0') != std::string::npos) { notes.push_back("Übersprungen (Binärdatei): " + relative.string()); continue; }
    for (const auto& comment : find_comments(content, is_hash_file(it->path()))) {
      ++comments;
      out << relative.generic_string() << ":" << comment.line << " | " << comment.text << "\n";
    }
  }
  std::ostringstream header;
  header << "Analyse abgeschlossen (keine Datei geändert).\nCode-Dateien geprüft: " << eligible << "\nGefundene Kommentare: " << comments << "\n\nDatei:Zeile | Kommentar\n";
  if (comments == 0) header << "Keine unterstützten Kommentare gefunden.\n";
  if (!notes.empty()) { header << "\nHinweise:"; for (const auto& note : notes) header << "\n- " << note; header << "\n"; }
  return header.str() + (comments == 0 ? std::string{} : "\n" + out.str());
}

std::vector<Change> scan_project(const fs::path& root, std::size_t& eligible, std::size_t& recognized, std::vector<std::string>& notes) {
  std::vector<Change> changes;
  for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied), end; it != end; ++it) {
    if (it->is_directory() && skip_directory(it->path())) { it.disable_recursion_pending(); continue; }
    if (!it->is_regular_file() || kExtensions.count(it->path().extension().string()) == 0) continue;
    ++eligible;
    if (it->file_size() > 5 * 1024 * 1024) { notes.push_back("Übersprungen (>5 MB): " + fs::relative(it->path(), root).string()); continue; }
    const std::string original = read_file(it->path());
    if (original.find('\0') != std::string::npos) { notes.push_back("Übersprungen (Binärdatei): " + fs::relative(it->path(), root).string()); continue; }
    const auto transformed = translate_comments(original, is_hash_file(it->path()));
    recognized += transformed.comments;
    if (transformed.text != original) changes.push_back({it->path(), original, transformed.text, transformed.comments});
  }
  return changes;
}
std::string project_report(const fs::path& folder, bool apply) {
  if (!fs::is_directory(folder)) throw std::runtime_error("choose an existing project folder");
  if (!apply) return analysis_report(folder);
  std::size_t eligible = 0, comments = 0; std::vector<std::string> notes;
  auto changes = scan_project(folder, eligible, comments, notes);
  std::ostringstream out;
  {
    const fs::path backup = folder / ".lunari-comment-backups" / timestamp();
    for (const auto& change : changes) {
      const fs::path relative = fs::relative(change.file, folder);
      fs::create_directories((backup / relative).parent_path());
      fs::copy_file(change.file, backup / relative, fs::copy_options::overwrite_existing);
    }
    if (!changes.empty()) engine->save(state_path);
    for (const auto& change : changes) write_file(change.file, change.translated);
    out << "Kommentare nach Lunari übersetzt.\nGeänderte Dateien: " << changes.size() << "\nÜbersetzte Kommentare: " << comments << "\nSicherung: " << backup.string() << "\n\nJeder übersetzte Kommentar beginnt mit 'Lunari:' und wird bei erneutem Lauf nicht nochmals übersetzt.";
  }
  if (!notes.empty()) { out << "\n\nHinweise:"; for (const auto& note : notes) out << "\n- " << note; }
  return out.str();
}
std::string choose_folder(HWND hwnd) {
  BROWSEINFOW dialog{}; dialog.hwndOwner = hwnd; dialog.lpszTitle = L"Projektordner auswählen"; dialog.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&dialog);
  if (!item) return {};
  wchar_t path[MAX_PATH]{}; const bool ok = SHGetPathFromIDListW(item, path) != FALSE; CoTaskMemFree(item);
  return ok ? wide_to_utf8(path) : std::string{};
}
void layout(HWND hwnd) {
  RECT client{}; GetClientRect(hwnd, &client); const int width = client.right, height = client.bottom, margin = 18;
  MoveWindow(GetDlgItem(hwnd, kPath), margin, 20, std::max(100, width - 400), 30, TRUE);
  MoveWindow(GetDlgItem(hwnd, kFolder), std::max(margin, width - 372), 20, 165, 30, TRUE);
  MoveWindow(GetDlgItem(hwnd, kAnalyze), std::max(margin, width - 197), 20, 80, 30, TRUE);
  MoveWindow(GetDlgItem(hwnd, kTranslate), std::max(margin, width - 107), 20, 89, 30, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStatus), margin, 65, std::max(100, width - margin * 2), 58, TRUE);
  MoveWindow(GetDlgItem(hwnd, kLog), margin, 135, std::max(100, width - margin * 2), std::max(100, height - 153), TRUE);
}
LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == kStateLoaded) {
    const auto* error = reinterpret_cast<std::string*>(wparam);
    if (error) {
      set_text(hwnd, kStatus, "Der Lunari-Datenstand konnte nicht geladen werden: " + *error);
      delete error;
    } else {
      state_ready = true;
      EnableWindow(GetDlgItem(hwnd, kFolder), TRUE);
      EnableWindow(GetDlgItem(hwnd, kAnalyze), TRUE);
      EnableWindow(GetDlgItem(hwnd, kTranslate), TRUE);
      set_text(hwnd, kStatus, "Lunari-Datenstand geladen. Projektordner wählen, analysieren und danach bei Bedarf übersetzen.");
    }
    return 0;
  }
  if (msg == WM_SIZE) { layout(hwnd); return 0; }
  if (msg == WM_COMMAND) {
    try {
      const int id = LOWORD(wparam);
      if (!state_ready) throw std::runtime_error("Lunari-Datenstand wird noch geladen");
      if (id == kFolder) { const std::string folder = choose_folder(hwnd); if (!folder.empty()) set_text(hwnd, kPath, folder); }
      if (id == kAnalyze || id == kTranslate) {
        const fs::path folder = control_text(hwnd, kPath);
        set_text(hwnd, kStatus, id == kAnalyze ? "Analysiere Kommentare außerhalb von Zeichenketten ..." : "Übersetze Kommentare und erstelle Sicherung ...");
        UpdateWindow(hwnd);
        const std::string result = project_report(folder, id == kTranslate);
        set_text(hwnd, kLog, result); set_text(hwnd, kStatus, id == kAnalyze ? "Analyse bereit. Keine Datei wurde geändert." : "Fertig. Originale liegen im Sicherungsordner.");
      }
    } catch (const std::exception& ex) { set_text(hwnd, kStatus, std::string("Vorgang fehlgeschlagen: ") + ex.what()); }
    return 0;
  }
  if (msg == WM_DESTROY) { if (state_loader.joinable()) state_loader.detach(); PostQuitMessage(0); return 0; }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
  const wchar_t klass[] = L"LunariDeveloperComments";
  WNDCLASSW wc{}; wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = klass; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&wc)) { MessageBoxW(nullptr, L"Fensterklasse konnte nicht registriert werden.", L"Lunari Entwickler-Kommentare", MB_ICONERROR); return 1; }
  HWND window = CreateWindowExW(0, klass, L"Lunari — Entwickler-Kommentare", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 920, 620, nullptr, nullptr, instance, nullptr);
  if (!window) { MessageBoxW(nullptr, L"Das Programmfenster konnte nicht erstellt werden.", L"Lunari Entwickler-Kommentare", MB_ICONERROR); return 1; }
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 18, 20, 450, 30, window, reinterpret_cast<HMENU>(kPath), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Projektordner wählen ...", WS_CHILD | WS_VISIBLE | WS_DISABLED, 480, 20, 165, 30, window, reinterpret_cast<HMENU>(kFolder), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Analysieren", WS_CHILD | WS_VISIBLE | WS_DISABLED, 655, 20, 80, 30, window, reinterpret_cast<HMENU>(kAnalyze), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Übersetzen", WS_CHILD | WS_VISIBLE | WS_DISABLED, 745, 20, 89, 30, window, reinterpret_cast<HMENU>(kTranslate), instance, nullptr);
  CreateWindowW(L"STATIC", L"Lunari-Datenstand wird geladen. Das Fenster bleibt bedienbar, sobald der große Wortbestand bereit ist.", WS_CHILD | WS_VISIBLE, 18, 65, 850, 58, window, reinterpret_cast<HMENU>(kStatus), instance, nullptr);
  CreateWindowW(L"EDIT", L"1. Projektordner auswählen\r\n2. Analysieren: Vorschau ohne Änderungen\r\n3. Übersetzen: Kommentare werden als 'Lunari: ...' ersetzt und Originale unter .lunari-comment-backups gesichert.", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 18, 135, 860, 430, window, reinterpret_cast<HMENU>(kLog), instance, nullptr);
  layout(window); ShowWindow(window, show); UpdateWindow(window);
  state_loader = std::thread([window] {
    try {
      state_path = preferred_state(executable_project_root());
      auto loaded = std::make_unique<leng::ResearchEngine>();
      loaded->load(state_path);
      engine = std::move(loaded);
      PostMessageW(window, kStateLoaded, 0, 0);
    } catch (const std::exception& ex) {
      PostMessageW(window, kStateLoaded, reinterpret_cast<WPARAM>(new std::string(ex.what())), 0);
    }
  });
  MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
  return 0;
}
