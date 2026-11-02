#include "leng/engine.hpp"
#include "leng/compute.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
void usage() {
  std::cout << "LINGUISTISCHE EMERGENZ NG\n"
               "Usage:\n"
               "  linguistische-emergenz-ng --self-test\n"
               "  linguistische-emergenz-ng --compute-test\n"
               "  linguistische-emergenz-ng --run <state.leng> [generations]\n"
               "  linguistische-emergenz-ng --run-local-lexicon <state.leng> [generations]\n"
               "  linguistische-emergenz-ng --resume <state.leng> [generations]\n"
               "  linguistische-emergenz-ng --report <state.leng>\n"
               "  linguistische-emergenz-ng --communication <state.leng>\n"
               "  linguistische-emergenz-ng --evolution <state.leng>\n"
               "  linguistische-emergenz-ng --population <state.leng>\n"
               "  linguistische-emergenz-ng --human-lab <state.leng>\n"
               "  linguistische-emergenz-ng --lexicon <state.leng>\n"
               "  linguistische-emergenz-ng --concept-communication <state.leng>\n"
               "  linguistische-emergenz-ng --productive-grammar <state.leng>\n"
               "  linguistische-emergenz-ng --phonotactics <state.leng>\n"
               "  linguistische-emergenz-ng --resave-state <state.leng>\n"
               "  linguistische-emergenz-ng --import-concepts <state.leng> <concepts.tsv>\n"
               "  linguistische-emergenz-ng --import-batch <state.leng> <concepts.tsv> [rows]\n"
               "  linguistische-emergenz-ng --import-german-batch <state.leng> <words.txt> [words]\n"
               "  linguistische-emergenz-ng --autonomous-step <state.leng> <concepts.tsv> [rows]\n"
               "  linguistische-emergenz-ng --lexicon-status <state.leng>\n"
               "  linguistische-emergenz-ng --search-lexicon <state.leng> <query>\n"
               "  linguistische-emergenz-ng --lexicon-benchmark <state.leng>\n"
               "  linguistische-emergenz-ng --translate <state.leng> <de|en|constructed> <text>\n"
               "  linguistische-emergenz-ng --translate-text <state.leng> <de|en|constructed> <text>\n"
               "  linguistische-emergenz-ng --translate-meaning <state.leng> <de|en|constructed> <text>\n"
               "  linguistische-emergenz-ng --compare <left.leng> <right.leng>\n"
               "  linguistische-emergenz-ng --export <state.leng> <directory>\n";
}
}

int main(int argc, char** argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--self-test") {
      std::string error;
      if (!leng::self_test(error)) { std::cerr << "SELF-TEST FAILED: " << error << '\n'; return 1; }
      std::cout << "SELF-TEST PASSED\n";
      return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--compute-test") {
      const auto parity = leng::run_reward_parity_test();
      std::cout << parity.detail << "\nmax_absolute_error=" << parity.max_absolute_error << '\n';
      return parity.skipped || parity.passed ? 0 : 1;
    }
    if (argc == 4 && std::string(argv[1]) == "--export") {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      engine.export_results(argv[3]);
      std::cout << "Export completed: " << argv[3] << '\n';
      return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--compare") {
      std::cout << leng::compare_research_states(argv[2], argv[3]) << '\n';
      return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--import-concepts") {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      const auto imported = engine.import_concepts(argv[3]);
      engine.save(argv[2]);
      std::cout << "Imported " << imported << " concepts and saved " << argv[2] << '\n';
      return 0;
    }
    if ((argc == 4 || argc == 5) && (std::string(argv[1]) == "--import-batch" || std::string(argv[1]) == "--autonomous-step")) {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      const std::size_t rows = argc == 5 ? static_cast<std::size_t>(std::stoull(argv[4])) : 1000;
      const auto progress = engine.import_concepts_batch(argv[3], rows);
      engine.save(argv[2]);
      std::cout << "Autonomous intake step\nsource-line=" << progress.source_line << "\naccepted=" << progress.rows_accepted << "\ncreated=" << progress.concepts_created << "\nenriched=" << progress.concepts_enriched << "\nrejected=" << progress.concepts_rejected << "\nrelations-created=" << progress.relations_created << "\ncomplete=" << (progress.complete ? "yes" : "no") << "\nfingerprint=" << progress.source_fingerprint << "\nstatus=" << progress.status << '\n';
      return 0;
    }
    if ((argc == 4 || argc == 5) && std::string(argv[1]) == "--import-german-batch") {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      const std::size_t words = argc == 5 ? static_cast<std::size_t>(std::stoull(argv[4])) : 50000;
      const auto progress = engine.import_german_word_list_batch(argv[3], words);
      engine.save(argv[2]);
      std::cout << "Local German word-list step\nsource-line=" << progress.source_line << "\naccepted=" << progress.rows_accepted << "\ncreated=" << progress.concepts_created << "\ncomplete=" << (progress.complete ? "yes" : "no") << "\nstatus=" << progress.status << '\n';
      return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--lexicon-status") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.lexicon_status() << '\n'; return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--search-lexicon") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.search_lexicon(argv[3]) << '\n'; return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--lexicon-benchmark") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.lexicon_benchmark() << '\n'; return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--resave-state") {
      leng::ResearchEngine engine; engine.load(argv[2]); engine.save(argv[2]); std::cout << "State migrated and saved: " << argv[2] << '\n'; return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--translate") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.translate(argv[4], argv[3]) << '\n'; return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--translate-text") {
      leng::ResearchEngine engine; engine.load(argv[2]); const std::string report = engine.translate_text(argv[4], argv[3]); if (std::string(argv[3]) == "constructed" && report.find("newly learned tokens=0") == std::string::npos) engine.save(argv[2]); std::cout << report << '\n'; return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--translate-meaning") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.translate_meaning(argv[4], argv[3]) << '\n'; return 0;
    }
    if (argc < 3) { usage(); return 2; }
    const std::string command = argv[1];
    const std::filesystem::path path = argv[2];
    leng::RunConfig config;
    if (argc == 4) config.generations = std::stoi(argv[3]);
    if (command == "--run-local-lexicon") config.seed_concept_lexicon = false;
    leng::ResearchEngine engine(config);
    if (command == "--resume" || command == "--report" || command == "--communication" || command == "--evolution" || command == "--population" || command == "--human-lab" || command == "--lexicon" || command == "--concept-communication" || command == "--productive-grammar" || command == "--phonotactics") engine.load(path);
    if (command == "--run" || command == "--run-local-lexicon" || command == "--resume") { engine.run(); engine.save(path); }
    if (command != "--run" && command != "--run-local-lexicon" && command != "--resume" && command != "--report" && command != "--communication" && command != "--evolution" && command != "--population" && command != "--human-lab" && command != "--lexicon" && command != "--concept-communication" && command != "--productive-grammar" && command != "--phonotactics") { usage(); return 2; }
    if (command == "--communication") std::cout << engine.communication_trace() << '\n';
    else if (command == "--evolution") std::cout << engine.evolution_trace() << '\n';
    else if (command == "--population") std::cout << engine.population_trace() << '\n';
    else if (command == "--human-lab") std::cout << engine.human_lab_trace() << '\n';
    else if (command == "--lexicon") std::cout << engine.lexicon_trace() << '\n';
    else if (command == "--concept-communication") std::cout << engine.concept_communication_trace() << '\n';
    else if (command == "--productive-grammar") std::cout << engine.productive_grammar_trace() << '\n';
    else if (command == "--phonotactics") std::cout << engine.phonotactic_trace() << '\n';
    else std::cout << engine.report() << "\n\n" << engine.export_language() << '\n';
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FATAL: " << e.what() << '\n';
    return 1;
  }
}
