#include "leng/engine.hpp"
#include "leng/lunari_grammar.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace {
constexpr int kRun = 1001;
constexpr int kSave = 1002;
constexpr int kLoad = 1003;
constexpr int kLanguage = 1004;
constexpr int kQuiz = 1005;
constexpr int kCheck = 1006;
constexpr int kCommunication = 1007;
constexpr int kEvolution = 1008;
constexpr int kPopulation = 1009;
constexpr int kExport = 1010;
constexpr int kCompare = 1011;
constexpr int kLexicon = 1012;
constexpr int kImport = 1013;
constexpr int kConceptCommunication = 1014;
constexpr int kLexiconStatus = 1015;
constexpr int kSearchLexicon = 1016;
constexpr int kLexiconBenchmark = 1017;
constexpr int kTranslateGerman = 1018;
constexpr int kTranslateEnglish = 1019;
constexpr int kTranslateConstructed = 1020;
constexpr int kToggleLanguage = 1021;
constexpr int kDictionary = 1022;
constexpr int kDictionaryPrevious = 1023;
constexpr int kDictionaryNext = 1024;
constexpr int kLunariGenerate = 1025;
constexpr int kLunariAnalyse = 1026;
constexpr int kLunariGrammar = 1027;
constexpr int kLunariWord = 1028;
constexpr int kOutput = 2001;
constexpr int kAnswer = 2002;
std::unique_ptr<leng::ResearchEngine> engine;
leng::HumanExercise exercise;
std::filesystem::path state_file{"leng-ui-state.leng"};
bool german_ui = true;
std::string dictionary_query;
std::size_t dictionary_page_index{};

const wchar_t* ui_text(const wchar_t* german, const wchar_t* english) {
  return german_ui ? german : english;
}

void set_control_text(HWND hwnd, int id, const wchar_t* german, const wchar_t* english) {
  SetWindowTextW(GetDlgItem(hwnd, id), ui_text(german, english));
}

void refresh_ui_language(HWND hwnd) {
  SetWindowTextW(hwnd, ui_text(L"LINGUISTISCHE EMERGENZ NG — Forschungsoberfläche",
                               L"LINGUISTIC EMERGENCE NG — Research Interface"));
  set_control_text(hwnd, kRun, L"Generation ausführen", L"Run generation");
  set_control_text(hwnd, kSave, L"Zustand speichern", L"Save state");
  set_control_text(hwnd, kLoad, L"Zustand laden", L"Load state");
  set_control_text(hwnd, kLanguage, L"Sprachlabor", L"Language lab");
  set_control_text(hwnd, kQuiz, L"Lernübung", L"Human exercise");
  set_control_text(hwnd, kCheck, L"Antwort prüfen", L"Check answer");
  set_control_text(hwnd, kCommunication, L"Kommunikation", L"Communication");
  set_control_text(hwnd, kEvolution, L"Evolution", L"Evolution");
  set_control_text(hwnd, kPopulation, L"Population", L"Population");
  set_control_text(hwnd, kExport, L"Export", L"Export");
  set_control_text(hwnd, kCompare, L"Zustand vergleichen", L"Compare typed state");
  set_control_text(hwnd, kLexicon, L"Lexikonlabor", L"Lexicon lab");
  set_control_text(hwnd, kImport, L"TSV importieren...", L"Import TSV...");
  set_control_text(hwnd, kConceptCommunication, L"Konzeptphrase", L"Concept phrase");
  set_control_text(hwnd, kLexiconStatus, L"Lexikonstatus", L"Lexicon status");
  set_control_text(hwnd, kSearchLexicon, L"Eingabe suchen", L"Search input");
  set_control_text(hwnd, kLexiconBenchmark, L"Messung", L"Benchmark");
  set_control_text(hwnd, kTranslateGerman, L"Übersetzen DE", L"Translate DE");
  set_control_text(hwnd, kTranslateEnglish, L"Übersetzen EN", L"Translate EN");
  set_control_text(hwnd, kTranslateConstructed, L"In Kunstsprache", L"To language");
  set_control_text(hwnd, kToggleLanguage, german_ui ? L"English" : L"Deutsch", german_ui ? L"English" : L"Deutsch");
  set_control_text(hwnd, kDictionary, L"Wörterbuch", L"Dictionary");
  set_control_text(hwnd, kDictionaryPrevious, L"← Seite", L"← Page");
  set_control_text(hwnd, kDictionaryNext, L"Seite →", L"Page →");
  set_control_text(hwnd, kLunariGenerate, L"Neu-Lunari erzeugen", L"Generate Neu-Lunari");
  set_control_text(hwnd, kLunariAnalyse, L"Neu-Lunari analysieren", L"Analyse Neu-Lunari");
  set_control_text(hwnd, kLunariGrammar, L"Neu-Lunari-Regeln", L"Neu-Lunari rules");
  set_control_text(hwnd, kLunariWord, L"Wort prüfen", L"Check word");
}

void set_output(HWND hwnd, const std::string& text) {
  std::string windows_text;
  windows_text.reserve(text.size() + 32);
  for (const char character : text) {
    if (character == '\n' && (windows_text.empty() || windows_text.back() != '\r')) windows_text += '\r';
    windows_text += character;
  }
  SetWindowTextA(GetDlgItem(hwnd, kOutput), windows_text.c_str());
}
std::string answer(HWND hwnd) { char buffer[512]{}; GetWindowTextA(GetDlgItem(hwnd, kAnswer), buffer, static_cast<int>(sizeof(buffer))); return buffer; }
std::string choose_tsv(HWND hwnd) {
  char path[MAX_PATH]{};
  OPENFILENAMEA dialog{};
  dialog.lStructSize = sizeof(dialog);
  dialog.hwndOwner = hwnd;
  dialog.lpstrFilter = "Concept TSV (*.tsv)\0*.tsv\0All files (*.*)\0*.*\0";
  dialog.lpstrFile = path;
  dialog.nMaxFile = static_cast<DWORD>(sizeof(path));
  dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  return GetOpenFileNameA(&dialog) ? std::string(path) : std::string{};
}
std::string choose_state(HWND hwnd) {
  char path[MAX_PATH]{};
  OPENFILENAMEA dialog{};
  dialog.lStructSize = sizeof(dialog);
  dialog.hwndOwner = hwnd;
  dialog.lpstrFilter = "Research state (*.leng)\0*.leng\0All files (*.*)\0*.*\0";
  dialog.lpstrFile = path;
  dialog.nMaxFile = static_cast<DWORD>(sizeof(path));
  dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  return GetOpenFileNameA(&dialog) ? std::string(path) : std::string{};
}
void ensure_engine() { if (!engine) { leng::RunConfig c; c.generations = 1; engine = std::make_unique<leng::ResearchEngine>(c); } }
void show_status(HWND hwnd, const std::string& tail = {}) { ensure_engine(); set_output(hwnd, engine->report() + (tail.empty() ? "" : "\r\n\r\n" + tail)); }

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_COMMAND) {
      const int id = LOWORD(wparam);
    try {
      if (id == kToggleLanguage) { german_ui = !german_ui; refresh_ui_language(hwnd); return 0; }
      if (id == kRun) { ensure_engine(); engine->run(); show_status(hwnd, "Experiment completed using actual engine state."); }
      if (id == kSave) { ensure_engine(); engine->save(state_file); show_status(hwnd, "Saved to " + state_file.string() + "."); }
      if (id == kLoad) {
        ensure_engine(); std::string requested = answer(hwnd);
        if (requested.empty()) { requested = choose_state(hwnd); if (requested.empty()) return 0; SetWindowTextA(GetDlgItem(hwnd, kAnswer), requested.c_str()); }
        state_file = requested;
        engine->load(state_file);
        show_status(hwnd, "Loaded " + state_file.string() + ".");
      }
      if (id == kLanguage) { ensure_engine(); set_output(hwnd, engine->export_language()); }
      if (id == kLexicon) { ensure_engine(); set_output(hwnd, engine->lexicon_trace()); }
      if (id == kLexiconStatus) { ensure_engine(); set_output(hwnd, engine->lexicon_status()); }
      if (id == kSearchLexicon) { ensure_engine(); set_output(hwnd, engine->search_lexicon(answer(hwnd))); }
      if (id == kDictionary) { ensure_engine(); dictionary_query = answer(hwnd); dictionary_page_index = 0; set_output(hwnd, engine->dictionary_page(dictionary_query, dictionary_page_index)); }
      if (id == kDictionaryPrevious) { ensure_engine(); if (dictionary_page_index > 0) --dictionary_page_index; set_output(hwnd, engine->dictionary_page(dictionary_query, dictionary_page_index)); }
      if (id == kDictionaryNext) { ensure_engine(); ++dictionary_page_index; set_output(hwnd, engine->dictionary_page(dictionary_query, dictionary_page_index)); }
      if (id == kLexiconBenchmark) { ensure_engine(); set_output(hwnd, engine->lexicon_benchmark()); }
      if (id == kTranslateGerman) { ensure_engine(); set_output(hwnd, engine->translate(answer(hwnd), "de")); }
      if (id == kTranslateEnglish) { ensure_engine(); set_output(hwnd, engine->translate(answer(hwnd), "en")); }
      if (id == kTranslateConstructed) { ensure_engine(); set_output(hwnd, engine->translate(answer(hwnd), "constructed")); }
      if (id == kLunariGenerate) {
        const auto specification = leng::parse_lunari_spec(answer(hwnd));
        const auto result = specification.ok ? leng::generate_lunari(specification.clause) : specification;
        set_output(hwnd, result.ok ? "Neu-Lunari v1 (vorgeschlagen):\r\n" + result.text + "\r\n\r\n" + leng::lunari_clause_trace(result.clause) : "Neu-Lunari-Diagnose: " + result.diagnostic);
      }
      if (id == kLunariAnalyse) {
        const auto result = leng::analyse_lunari(answer(hwnd));
        set_output(hwnd, result.ok ? result.text + "\r\n\r\n" + leng::lunari_clause_trace(result.clause) : "Neu-Lunari-Diagnose: " + result.diagnostic);
      }
      if (id == kLunariGrammar) set_output(hwnd, leng::lunari_grammar_reference());
      if (id == kLunariWord) {
        const auto word = leng::assess_neu_lunari_word(answer(hwnd));
        set_output(hwnd, word.valid ? "Neu-Lunari v1 — Lautprüfung\r\nWortform: gültig\r\nSilben: " + word.syllables + "\r\nBetonung: regulär vorletzte Silbe (Akut nur für dokumentierte Abweichung)." : "Neu-Lunari-Diagnose: " + word.diagnostic);
      }
      if (id == kImport) { ensure_engine(); std::string import_path = answer(hwnd); if (import_path.empty()) { import_path = choose_tsv(hwnd); if (import_path.empty()) return 0; SetWindowTextA(GetDlgItem(hwnd, kAnswer), import_path.c_str()); } const auto imported = engine->import_concepts(import_path); show_status(hwnd, "Imported " + std::to_string(imported) + " concepts. Press Save state to persist them."); }
      if (id == kQuiz) { ensure_engine(); exercise = engine->create_human_exercise(); SetWindowTextA(GetDlgItem(hwnd, kAnswer), ""); show_status(hwnd, exercise.prompt + "\r\nEnter the requested answer below and press Check answer."); }
      if (id == kCheck) { ensure_engine(); const bool correct = engine->check_human_exercise(exercise, answer(hwnd)); set_output(hwnd, std::string(correct ? "Human language lab: correct full-statement answer recorded.\r\n\r\n" : "Human language lab: answer not correct; review Language and retry.\r\n\r\n") + engine->human_lab_trace()); }
      if (id == kCommunication) { ensure_engine(); set_output(hwnd, engine->communication_trace()); }
      if (id == kConceptCommunication) { ensure_engine(); set_output(hwnd, engine->concept_communication_trace()); }
      if (id == kEvolution) { ensure_engine(); set_output(hwnd, engine->evolution_trace()); }
      if (id == kPopulation) { ensure_engine(); set_output(hwnd, engine->population_trace()); }
      if (id == kExport) { ensure_engine(); engine->export_results("leng-export"); show_status(hwnd, "Exported language, report, communication, population and Pareto archive to leng-export."); }
      if (id == kCompare) {
        ensure_engine();
        const std::string reference = answer(hwnd);
        if (reference.empty()) throw std::invalid_argument("enter a saved .leng path before Compare");
        const std::filesystem::path current = "leng-ui-current-compare.leng";
        engine->save(current);
        try { set_output(hwnd, leng::compare_research_states(current, reference)); }
        catch (...) { std::filesystem::remove(current); throw; }
        std::filesystem::remove(current);
      }
    } catch (const std::exception& ex) {
      const std::string prefix = german_ui ? "Vorgang fehlgeschlagen: " : "Operation failed: ";
      const std::string help = id == kLoad
          ? (german_ui ? "\n\nBitte wähle eine vorhandene .leng-Forschungsdatei aus. Bei leerem Eingabefeld öffnet Zustands laden die Dateiauswahl."
                       : "\n\nChoose an existing .leng research-state file. With an empty input field, Load state opens the file chooser.")
          : "";
      set_output(hwnd, prefix + ex.what() + help);
    }
    return 0;
  }
  if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
  const wchar_t klass[] = L"LinguistischeEmergenzNG";
  WNDCLASSW wc{}; wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = klass; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&wc)) return 1;
  HWND hwnd = CreateWindowExW(0, klass, L"LINGUISTISCHE EMERGENZ NG — Forschungsoberfläche", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 920, 660, nullptr, nullptr, instance, nullptr);
  if (!hwnd) return 1;
  CreateWindowW(L"BUTTON", L"Run generation", WS_CHILD | WS_VISIBLE, 15, 15, 125, 30, hwnd, reinterpret_cast<HMENU>(kRun), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Save state", WS_CHILD | WS_VISIBLE, 150, 15, 100, 30, hwnd, reinterpret_cast<HMENU>(kSave), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Load state", WS_CHILD | WS_VISIBLE, 260, 15, 100, 30, hwnd, reinterpret_cast<HMENU>(kLoad), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Language lab", WS_CHILD | WS_VISIBLE, 370, 15, 115, 30, hwnd, reinterpret_cast<HMENU>(kLanguage), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Human exercise", WS_CHILD | WS_VISIBLE, 495, 15, 125, 30, hwnd, reinterpret_cast<HMENU>(kQuiz), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Communication", WS_CHILD | WS_VISIBLE, 630, 15, 115, 30, hwnd, reinterpret_cast<HMENU>(kCommunication), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Evolution", WS_CHILD | WS_VISIBLE, 755, 15, 100, 30, hwnd, reinterpret_cast<HMENU>(kEvolution), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Lexicon lab", WS_CHILD | WS_VISIBLE, 15, 88, 115, 28, hwnd, reinterpret_cast<HMENU>(kLexicon), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 15, 55, 480, 26, hwnd, reinterpret_cast<HMENU>(kAnswer), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Check answer", WS_CHILD | WS_VISIBLE, 505, 54, 115, 28, hwnd, reinterpret_cast<HMENU>(kCheck), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Population", WS_CHILD | WS_VISIBLE, 630, 54, 115, 28, hwnd, reinterpret_cast<HMENU>(kPopulation), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Export", WS_CHILD | WS_VISIBLE, 755, 54, 100, 28, hwnd, reinterpret_cast<HMENU>(kExport), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Compare typed state", WS_CHILD | WS_VISIBLE, 140, 88, 160, 28, hwnd, reinterpret_cast<HMENU>(kCompare), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Import TSV...", WS_CHILD | WS_VISIBLE, 310, 88, 115, 28, hwnd, reinterpret_cast<HMENU>(kImport), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Concept phrase", WS_CHILD | WS_VISIBLE, 435, 88, 125, 28, hwnd, reinterpret_cast<HMENU>(kConceptCommunication), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Lexicon status", WS_CHILD | WS_VISIBLE, 570, 88, 125, 28, hwnd, reinterpret_cast<HMENU>(kLexiconStatus), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Search input", WS_CHILD | WS_VISIBLE, 705, 88, 115, 28, hwnd, reinterpret_cast<HMENU>(kSearchLexicon), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Benchmark", WS_CHILD | WS_VISIBLE, 830, 88, 70, 28, hwnd, reinterpret_cast<HMENU>(kLexiconBenchmark), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Translate DE", WS_CHILD | WS_VISIBLE, 15, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kTranslateGerman), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Translate EN", WS_CHILD | WS_VISIBLE, 140, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kTranslateEnglish), instance, nullptr);
  CreateWindowW(L"BUTTON", L"To language", WS_CHILD | WS_VISIBLE, 265, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kTranslateConstructed), instance, nullptr);
  CreateWindowW(L"BUTTON", L"English", WS_CHILD | WS_VISIBLE, 390, 125, 100, 28, hwnd, reinterpret_cast<HMENU>(kToggleLanguage), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Dictionary", WS_CHILD | WS_VISIBLE, 500, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kDictionary), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Previous page", WS_CHILD | WS_VISIBLE, 625, 125, 100, 28, hwnd, reinterpret_cast<HMENU>(kDictionaryPrevious), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Next page", WS_CHILD | WS_VISIBLE, 735, 125, 100, 28, hwnd, reinterpret_cast<HMENU>(kDictionaryNext), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Generate Neu-Lunari", WS_CHILD | WS_VISIBLE, 15, 155, 160, 28, hwnd, reinterpret_cast<HMENU>(kLunariGenerate), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Analyse Neu-Lunari", WS_CHILD | WS_VISIBLE, 185, 155, 160, 28, hwnd, reinterpret_cast<HMENU>(kLunariAnalyse), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Neu-Lunari rules", WS_CHILD | WS_VISIBLE, 355, 155, 145, 28, hwnd, reinterpret_cast<HMENU>(kLunariGrammar), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Check word", WS_CHILD | WS_VISIBLE, 510, 155, 120, 28, hwnd, reinterpret_cast<HMENU>(kLunariWord), instance, nullptr);
  HWND output = CreateWindowW(L"EDIT", L"Run a generation to create a language candidate.", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 15, 195, 870, 410, hwnd, reinterpret_cast<HMENU>(kOutput), instance, nullptr);
  SendMessageW(output, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
  refresh_ui_language(hwnd);
  ShowWindow(hwnd, show);
  MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
  return 0;
}
