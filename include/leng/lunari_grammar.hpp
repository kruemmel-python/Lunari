#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

namespace leng {

// Neu-Lunari v1 is deliberately separate from the reversible source-text
// channel.  Values are stable semantic labels; surface forms are versioned by
// the grammar implementation.
struct LunariClause {
  std::string speech_act{"statement"}; // statement | question | request | prohibition | answer-yes | answer-no
  std::string subject;
  std::string verb;
  std::string object;
  std::string determiner;  // definite | indefinite | general
  // plural | exact:N | about:N | atleast:N | atmost:N | fraction:A/B | ordinal:N
  std::string quantity;
  std::string measure;     // optional invariant measure/unit after quantity
  std::string adjective;
  std::string predicate_adjective; // property after the proposed copula
  std::string degree;       // empty | very | more | less | equal | most
  std::string compared_with; // explicit comparison standard
  std::string time;        // present | past | future
  std::string aspect;      // plain | progressive | completed | habitual
  std::string modality;    // empty | ability | possibility | permission | obligation | recommendation | intent | wish
  std::string negation;    // none | predicate | statement
  std::string relation;    // empty | location | destination | source | recipient | instrument | companion | topic | purpose | possession | part | content | material
  std::string related;
  std::string question_word; // person | thing | place | destination | source | time | reason | manner | amount | choice
  std::string focus_negation; // empty | subject | object | modality
  std::string probability;    // empty | probable | uncertain
  std::string emotion;        // optional lexical emotion under explicit scope marker
  std::string social_relation; // optional lexical social relation under explicit scope marker
  std::string goal;           // optional lexical communicative goal under explicit scope marker
  std::string statement_level; // empty | report | hypothesis | fiction | quote
  std::string connective;      // empty | and | or | but | cause | condition | concession | purpose | temporal
  std::string embedding;       // empty | content | relative | quote
  std::shared_ptr<LunariClause> embedded;
  bool passive{};
};

struct LunariGrammarResult {
  bool ok{};
  LunariClause clause;
  std::string text;
  std::string diagnostic;
};

struct LunariTextResult {
  bool ok{};
  std::string text;
  std::string diagnostic;
};

struct LunariWordAssessment {
  bool valid{};
  std::string syllables;
  std::string diagnostic;
};
struct LunariDerivationResult {
  bool ok{};
  std::string form;
  std::string diagnostic;
};

// Semicolon-separated direct meaning input, for example:
// subject=I;verb=see;object=sun;time=past;negation=predicate
[[nodiscard]] LunariGrammarResult parse_lunari_spec(const std::string& specification);
// Bounded German interface for the end-user translator.  It recognises the
// documented Neu-Lunari vocabulary and grammar; unsupported German is
// rejected rather than silently sent through the legacy reversible channel.
[[nodiscard]] LunariGrammarResult parse_german_lunari_sentence(const std::string& sentence);
[[nodiscard]] LunariGrammarResult generate_lunari(const LunariClause& clause);
[[nodiscard]] LunariGrammarResult analyse_lunari(const std::string& sentence);
[[nodiscard]] std::string render_lunari_clause_german(const LunariClause& clause);
[[nodiscard]] LunariTextResult translate_german_lunari_text(const std::string& text);
[[nodiscard]] LunariTextResult translate_lunari_german_text(const std::string& text);
// Deterministic, reversible content-lexicon codec used for the complete
// German source list.  It is independent from historical .leng states.
[[nodiscard]] std::string encode_german_neu_lunari_lexeme(const std::string& word);
[[nodiscard]] std::string decode_neu_lunari_german_lexeme(const std::string& word);
// Loads the v2 persistent German ↔ Neu-Lunari lexicon.  Older codec-only
// files are deliberately ignored because they are not part of the new lexicon.
void load_neu_lunari_lexicon(const std::filesystem::path& path);
[[nodiscard]] LunariWordAssessment assess_neu_lunari_word(const std::string& word);
// Productive formations are structurally interpretable, not automatically
// confirmed dictionary meanings. kind: agent | tool | abstract | field | compound.
[[nodiscard]] LunariDerivationResult derive_neu_lunari_word(const std::string& root, const std::string& kind, const std::string& qualifier = {});
// Learner-facing inspection of the structured sentence; no lexical meanings
// are invented for unknown content words.
[[nodiscard]] std::string lunari_clause_trace(const LunariClause& clause);
[[nodiscard]] std::string lunari_grammar_reference();
[[nodiscard]] std::vector<std::string> neu_lunari_reserved_forms();

} // namespace leng
