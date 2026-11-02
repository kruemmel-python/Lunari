#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace leng {

enum class SpeechAct { statement, question, request };

struct Meaning {
  SpeechAct act{SpeechAct::statement};
  std::string actor;
  std::string action;
  std::string object;
  std::string relation;
  bool negated{false};
  int time{0};
  int quantity{1};
  std::string modality{"certain"};
  std::string probability{"likely"};
  std::string emotion{"neutral"};
  std::string social_relation{"peer"};
  std::string goal{"inform"};
  std::string proposition_level{"direct"};
  std::string abstraction{"literal"};
  [[nodiscard]] std::string key() const;
  [[nodiscard]] bool operator==(const Meaning&) const = default;
};

struct Metrics {
  double communication_success{};
  double semantic_accuracy{};
  double cost{};
  double pronounceability{};
  double grammar_complexity{};
  double robustness{};
  double expressiveness{};
  [[nodiscard]] double score() const;
};

struct CandidateSummary {
  std::uint64_t id{};
  std::uint64_t parent_id{};
  Metrics metrics{};
  std::string sample;
};

struct HumanExercise {
  std::string prompt;
  std::string expected_answer;
  std::uint64_t issued_unix_ms{};
  bool interpretation{};
};

// Persisted outcome of one bounded intake operation.  The caller can save the
// engine after every batch and resume exactly at the recorded source line.
struct ImportProgress {
  std::size_t source_line{};
  std::size_t rows_seen{};
  std::size_t rows_accepted{};
  std::size_t concepts_created{};
  std::size_t concepts_enriched{};
  std::size_t concepts_rejected{};
  std::size_t relations_created{};
  bool complete{};
  std::string source_fingerprint;
  std::string status;
};

struct RunConfig {
  std::uint64_t seed{20261101};
  int population{12};
  int generations{12};
  int training_rounds{160};
  int evaluation_rounds{80};
  // A word-list-only state must not silently add demonstration concepts.
  bool seed_concept_lexicon{true};
};

class ResearchEngine {
 public:
  explicit ResearchEngine(RunConfig config = {});
  ~ResearchEngine();
  ResearchEngine(const ResearchEngine&) = delete;
  ResearchEngine& operator=(const ResearchEngine&) = delete;
  void run();
  void save(const std::filesystem::path& path) const;
  void load(const std::filesystem::path& path);
  // Imports UTF-8 TSV rows: label, part_of_speech, semantics, relations,
  // source, provenance. The source and provenance columns are mandatory.
  [[nodiscard]] std::size_t import_concepts(const std::filesystem::path& tsv_path);
  // Bounded, resumable variant for large local or pre-fetched Wikimedia TSV
  // sources.  `max_rows` counts data rows, never header/comment rows.
  [[nodiscard]] ImportProgress import_concepts_batch(const std::filesystem::path& tsv_path,
                                                      std::size_t max_rows);
  // Imports a local German word-form list one line at a time. These entries
  // are kept separate from externally attested concepts: their meanings and
  // licence are intentionally marked unverified unless supplied elsewhere.
  [[nodiscard]] ImportProgress import_german_word_list_batch(const std::filesystem::path& word_list_path,
                                                              std::size_t max_words);
  // Runs independent sender/receiver checks for language-realised concepts
  // that have not yet received the requested number of trials.
  [[nodiscard]] std::size_t validate_concepts(std::size_t max_concepts = 0);
  void export_results(const std::filesystem::path& directory) const;
  [[nodiscard]] std::string report() const;
  [[nodiscard]] const std::vector<CandidateSummary>& archive() const;
  [[nodiscard]] std::string export_language() const;
  [[nodiscard]] std::string lexicon_trace() const;
  [[nodiscard]] std::string lexicon_status() const;
  [[nodiscard]] std::string search_lexicon(const std::string& query, std::size_t limit = 50) const;
  // Human-readable, paginated dictionary for the persistent concept forms.
  [[nodiscard]] std::string dictionary_page(const std::string& query, std::size_t page,
                                            std::size_t page_size = 20) const;
  [[nodiscard]] std::string lexicon_benchmark() const;
  // Translates through the persistent semantic lexicon. `target_language` is
  // `de`, `en`, or `constructed`; input language is detected from known forms.
  // Unknown labels are reported explicitly instead of being invented.
  [[nodiscard]] std::string translate(const std::string& text,
                                      const std::string& target_language) const;
  // Structure-preserving text channel. Unlike the semantic lab translator it
  // keeps punctuation, whitespace, case, dialogue marks, and unknown tokens.
  [[nodiscard]] std::string translate_text(const std::string& text,
                                           const std::string& target_language);
  // Strict meaning channel for the reviewed semantic core. It encodes a
  // language-independent Meaning frame rather than source word forms. A
  // source sentence is rejected when actor, action, or object cannot be
  // identified; no unknown meaning is guessed.
  [[nodiscard]] std::string translate_meaning(const std::string& text,
                                              const std::string& target_language) const;
  [[nodiscard]] std::string concept_communication_trace() const;
  [[nodiscard]] std::string productive_grammar_trace() const;
  [[nodiscard]] std::string phonotactic_trace() const;
  [[nodiscard]] std::string communication_trace() const;
  [[nodiscard]] std::string evolution_trace() const;
  [[nodiscard]] std::string population_trace() const;
  [[nodiscard]] HumanExercise create_human_exercise() const;
  [[nodiscard]] bool check_human_exercise(const HumanExercise& exercise, const std::string& answer);
  [[nodiscard]] std::string human_lab_trace() const;
  [[nodiscard]] int generation() const;

 private:
  struct Impl;
  Impl* impl_;
};

bool self_test(std::string& error);
[[nodiscard]] std::string compare_research_states(const std::filesystem::path& left,
                                                   const std::filesystem::path& right);

}  // namespace leng
