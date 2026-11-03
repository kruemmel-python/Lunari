#include "leng/engine.hpp"
#include "leng/compute.hpp"
#include "leng/lunari_grammar.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace leng {
namespace {
constexpr std::array<const char*, 5> kActors{"human", "machine", "child", "researcher", "agent"};
constexpr std::array<const char*, 5> kActions{"move", "give", "observe", "build", "ask"};
constexpr std::array<const char*, 5> kObjects{"stone", "signal", "map", "water", "idea"};
constexpr std::array<const char*, 4> kRelations{"near", "before", "because", "if"};
constexpr std::array<const char*, 3> kModalities{"certain", "possible", "required"};
constexpr std::array<const char*, 3> kProbabilities{"unlikely", "likely", "unknown"};
constexpr std::array<const char*, 3> kEmotions{"neutral", "joy", "concern"};
constexpr std::array<const char*, 3> kSocialRelations{"peer", "ally", "observer"};
constexpr std::array<const char*, 3> kGoals{"inform", "assist", "coordinate"};
constexpr std::array<const char*, 2> kPropositionLevels{"direct", "reported"};
constexpr std::array<const char*, 2> kAbstractions{"literal", "metaphoric"};
// Neu-Lunari favours compact CV(C) feet.  A coda is optional, but the small
// set below and the coda/onset table keep clusters recognisable rather than
// turning every word into an arbitrary consonant pile.
constexpr std::array<const char*, 14> kOnsets{"m", "n", "l", "r", "p", "t", "k", "b", "d", "g", "f", "s", "h", "v"};
constexpr std::array<const char*, 5> kVowels{"a", "e", "i", "o", "u"};
constexpr std::array<const char*, 7> kCodas{"m", "n", "l", "r", "s", "v", "d"};
constexpr const char* kConceptMorphologyRuleVersion = "morphology-v4-neu-lunari-compact-cvc;relation-compound-v2";

std::string act_name(SpeechAct act) {
  switch (act) { case SpeechAct::statement: return "statement"; case SpeechAct::question: return "question"; case SpeechAct::request: return "request"; }
  return "statement";
}
SpeechAct parse_act(const std::string& value) {
  if (value == "question") return SpeechAct::question;
  if (value == "request") return SpeechAct::request;
  return SpeechAct::statement;
}
std::vector<std::string> atoms(const Meaning& m) {
  return {"act:" + act_name(m.act), "actor:" + m.actor, "action:" + m.action,
          "object:" + m.object, "relation:" + m.relation, std::string("neg:") + (m.negated ? "1" : "0"),
          "time:" + std::to_string(m.time), "qty:" + std::to_string(m.quantity),
          "modality:" + m.modality, "probability:" + m.probability, "emotion:" + m.emotion,
          "social:" + m.social_relation, "goal:" + m.goal, "proposition:" + m.proposition_level, "abstraction:" + m.abstraction};
}
const std::vector<std::string>& default_order() {
  static const std::vector<std::string> value{"act", "actor", "action", "object", "relation", "neg", "time", "qty", "modality", "probability", "emotion", "social", "goal", "proposition", "abstraction"};
  return value;
}
Meaning random_meaning(std::mt19937_64& rng, bool held_out) {
  std::uniform_int_distribution<int> act(0, 2), actor(0, 4), action(0, 4), object(0, 4), relation(0, 3), bit(0, 1), time(0, 2), quantity(1, 3);
  std::uniform_int_distribution<int> modality(0, 2), probability(0, 2), emotion(0, 2), social(0, 2), goal(0, 2), proposition(0, 1), abstraction(0, 1);
  Meaning m{static_cast<SpeechAct>(act(rng)), kActors[actor(rng)], kActions[action(rng)], kObjects[object(rng)], kRelations[relation(rng)], bit(rng) != 0, time(rng), quantity(rng), kModalities[modality(rng)], kProbabilities[probability(rng)], kEmotions[emotion(rng)], kSocialRelations[social(rng)], kGoals[goal(rng)], kPropositionLevels[proposition(rng)], kAbstractions[abstraction(rng)]};
  // Evaluation combines known primitives in combinations deliberately excluded from training.
  if (held_out) { m.relation = "if"; m.quantity = 3; m.negated = m.action == "ask" || m.object == "idea"; m.modality = "required"; m.proposition_level = "reported"; m.abstraction = "metaphoric"; }
  return m;
}
bool is_vowel(char value) { return std::string_view{"aeiou"}.find(value) != std::string_view::npos; }
bool is_onset(char value) { return std::string_view{"mnlrptkbdgfshv"}.find(value) != std::string_view::npos; }
bool is_coda(char value) { return std::string_view{"mnlrsvd"}.find(value) != std::string_view::npos; }
bool allowed_coda_onset(char coda, char onset) {
  // Sonorant or sibilant codas may only meet a selected following onset.
  // This permits forms such as mem-kur while making the pattern predictable.
  return std::string_view{"mpmtmkmvnsntnknvlklvltrtrkrnrlmlvsmstskvrdrdv"}.find(std::string{coda} + onset) != std::string_view::npos;
}

struct PhonotacticScore { double structure{}; double transition{}; double length{}; double acoustic{}; double combined{}; bool compact_syllables{}; };
PhonotacticScore phonotactic_score(const std::string& word) {
  PhonotacticScore score{1.0, 1.0, 1.0, 1.0, 0.0, true};
  if (word.empty()) return {};
  std::vector<std::string> syllables;
  std::string unit;
  for (const char raw : word) {
    if (raw == '-') {
      if (unit.empty() || unit.back() == ' ') return {};
      unit.push_back(' ');
    } else {
      unit.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(raw))));
    }
  }
  if (unit.empty() || unit.back() == ' ') return {};
  std::size_t start = 0;
  while (start < unit.size()) {
    const std::size_t stop = unit.find(' ', start);
    const std::string part = unit.substr(start, stop == std::string::npos ? std::string::npos : stop - start);
    std::size_t pos = 0;
    while (pos < part.size()) {
      const std::size_t syllable_start = pos;
      if (is_onset(part[pos])) ++pos;
      if (pos >= part.size() || !is_vowel(part[pos])) return {};
      ++pos;
      if (pos < part.size() && is_coda(part[pos])) {
        const char coda = part[pos];
        if (pos + 1 == part.size()) {
          ++pos;
        } else if (is_onset(part[pos + 1]) && allowed_coda_onset(coda, part[pos + 1])) {
          ++pos;
        }
      }
      syllables.push_back(part.substr(syllable_start, pos - syllable_start));
    }
    start = stop == std::string::npos ? unit.size() : stop + 1;
  }
  // Repeated adjacent syllables remain possible but are penalised as
  // acoustically confusable. Closed feet receive a small reward because they
  // break the earlier sing-song CV-CV-CV rhythm.
  std::size_t closed{};
  for (const auto& syllable : syllables) if (is_coda(syllable.back())) ++closed;
  for (std::size_t i = 1; i < syllables.size(); ++i) {
    if (syllables[i] == syllables[i - 1]) { score.transition -= 0.14; score.acoustic -= 0.20; }
    if (syllables[i].back() == syllables[i - 1].back()) { score.transition -= 0.03; score.acoustic -= 0.04; }
    if (syllables[i].front() == syllables[i - 1].front()) score.acoustic -= 0.03;
  }
  const std::size_t count = syllables.size();
  score.length = count <= 2 ? 1.0 : count == 3 ? 0.98 : count == 4 ? 0.94 : count == 5 ? 0.86 : 0.72;
  if (closed == 0) score.acoustic -= 0.08;
  else if (closed * 2 > count) score.transition -= 0.05;
  score.transition = std::clamp(score.transition, 0.0, 1.0);
  score.acoustic = std::clamp(score.acoustic, 0.0, 1.0);
  score.combined = 0.30 * score.structure + 0.30 * score.transition + 0.20 * score.length + 0.20 * score.acoustic;
  return score;
}

std::string new_word(std::mt19937_64& rng, std::set<std::string>& used) {
  std::uniform_int_distribution<int> o(0, static_cast<int>(kOnsets.size() - 1)), v(0, static_cast<int>(kVowels.size() - 1)), c(0, static_cast<int>(kCodas.size() - 1)), syl(1, 2);
  for (;;) {
    std::string out;
    const int n = syl(rng);
    for (int i = 0; i < n; ++i) {
      out += std::string(kOnsets[o(rng)]) + kVowels[v(rng)];
      if ((i + 1 == n || (i == 0 && n == 2)) && (rng() % 100) < 65) out += kCodas[c(rng)];
    }
    if (phonotactic_score(out).combined >= 0.88 && used.insert(out).second) return out;
  }
}
double word_quality(const std::string& word) {
  return phonotactic_score(word).combined;
}

std::string new_concept_word(std::mt19937_64& rng, std::unordered_set<std::string>& used) {
  std::uniform_int_distribution<int> onset(0, static_cast<int>(kOnsets.size() - 1));
  std::uniform_int_distribution<int> vowel(0, static_cast<int>(kVowels.size() - 1));
  std::uniform_int_distribution<int> coda(0, static_cast<int>(kCodas.size() - 1));
  // Three compact syllables with optional codas retain a large form space
  // without forcing every root into the same four-CV cadence.
  for (;;) {
    std::string out;
    for (int i = 0; i < 3; ++i) {
      out += std::string(kOnsets[onset(rng)]) + kVowels[vowel(rng)];
      if ((i == 2 || (i == 0 && (rng() % 100) < 45)) && (rng() % 100) < 70) out += kCodas[coda(rng)];
    }
    if (phonotactic_score(out).combined >= 0.84 && used.insert(out).second) return out;
  }
}

struct Candidate {
  std::uint64_t id{};
  std::uint64_t parent{};
  std::map<std::string, std::string> lexicon;
  // A small, separate inventory of reusable morphology.  These forms are not
  // semantic-frame slots and therefore do not force every message to carry a
  // person/number/tense value.
  std::map<std::string, std::string> morphology;
  std::vector<std::string> order;
  Metrics metrics{};
  double selection_score{};
};
void ensure_morphology(Candidate& candidate, std::mt19937_64& rng) {
  if (candidate.morphology.size() == 5) return;
  std::set<std::string> used;
  for (const auto& [_, word] : candidate.lexicon) used.insert(word);
  for (const auto& [_, word] : candidate.morphology) used.insert(word);
  for (const std::string key : {"person:3", "number:sg", "number:pl", "time:present", "time:past"})
    if (!candidate.morphology.contains(key)) candidate.morphology.emplace(key, new_word(rng, used));
}
std::vector<std::string> ordered_atoms(const Candidate& candidate, const Meaning& meaning) {
  std::map<std::string, std::string> by_field;
  for (const auto& atom : atoms(meaning)) by_field.emplace(atom.substr(0, atom.find(':')), atom);
  const auto& order = candidate.order.empty() ? default_order() : candidate.order;
  std::vector<std::string> result;
  result.reserve(order.size());
  for (const auto& field : order) result.push_back(by_field.at(field));
  return result;
}

struct AgentState {
  std::uint64_t id{};
  std::map<std::string, std::string> private_reverse_lexicon;
  std::uint64_t successful_interpretations{};
  std::uint64_t failed_interpretations{};
};

// This is deliberately separate from Meaning's compact sentence frame.  A
// concept can be added without adding another compulsory slot to every
// utterance, and its history remains available after a research run.
struct ConceptEntry {
  std::string id;
  // Stable external identity where available (for example Wikidata Q42).
  // Locally-derived entries deliberately leave this empty.
  std::string external_id;
  std::string label;
  std::string part_of_speech;
  std::string form;
  std::string derivation;
  std::string source;
  std::string provenance;
  std::string source_revision;
  // Categories are explicit evidence classes, never inferred from a label.
  // external-attested | simulated-relation | agent-hypothesis
  std::string evidence_class;
  // pending | realised | communicatively-validated | rejected
  std::string language_status;
  std::string semantics;
  std::string relations;
  int introduced_generation{};
  std::uint64_t validation_attempts{};
  std::uint64_t validation_successes{};
  std::uint64_t validation_failures{};
};

struct ImportCheckpoint {
  std::string source_path;
  std::string fingerprint;
  std::size_t next_line{1};
  std::size_t rows_seen{};
  std::size_t rows_accepted{};
  std::size_t rows_rejected{};
  bool complete{};
  std::string status{"idle"};
};
struct GrammarLexeme {
  std::string concept_id, historic_form, form, language_version, status, source, note;
  std::string part_of_speech, meaning, valency, scope;
};

std::string join(const std::vector<std::string>& values, const char* separator = ",") {
  std::ostringstream out;
  for (const auto& value : values) { if (out.tellp() > 0) out << separator; out << value; }
  return out.str();
}

std::string trim(std::string value) {
  const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
  const auto begin = std::find_if(value.begin(), value.end(), not_space);
  const auto end = std::find_if(value.rbegin(), value.rend(), not_space).base();
  return begin >= end ? "" : std::string(begin, end);
}

std::string canonical_concept_key(std::string value) {
  value = trim(std::move(value));
  std::string out;
  bool pending_space = false;
  for (const unsigned char ch : value) {
    if (std::isspace(ch)) { pending_space = !out.empty(); continue; }
    if (pending_space) { out.push_back(' '); pending_space = false; }
    out.push_back(static_cast<char>(std::tolower(ch)));
  }
  return out;
}

std::vector<std::string> words(std::string value) {
  for (char& ch : value) {
    const unsigned char byte = static_cast<unsigned char>(ch);
    if (!std::isalnum(byte) && ch != '-' && byte < 128) ch = ' ';
  }
  std::istringstream in(canonical_concept_key(std::move(value)));
  std::vector<std::string> result;
  for (std::string word; in >> word;) result.push_back(std::move(word));
  return result;
}

bool text_token_byte(unsigned char value) {
  // UTF-8 bytes are retained together in a lexical token; only ASCII layout
  // and punctuation separate tokens. This keeps German umlauts and quotes.
  return value >= 128 || std::isalnum(value) || value == '_';
}
std::vector<std::pair<bool, std::string>> text_segments(const std::string& text) {
  std::vector<std::pair<bool, std::string>> result;
  std::string current; bool lexical = false;
  for (std::size_t index = 0; index < text.size();) {
    const unsigned char ch = static_cast<unsigned char>(text[index]);
    // German typographic quotation marks and dashes are separators, not
    // unknown words. Preserve their UTF-8 bytes exactly as layout text.
    std::size_t width = 1;
    bool next_lexical = text_token_byte(ch);
    if (ch == 0xE2 && index + 2 < text.size() && static_cast<unsigned char>(text[index + 1]) == 0x80) { width = 3; next_lexical = false; }
    if (ch == 0xC2 && index + 1 < text.size() && (static_cast<unsigned char>(text[index + 1]) == 0xAB || static_cast<unsigned char>(text[index + 1]) == 0xBB)) { width = 2; next_lexical = false; }
    if (!current.empty() && next_lexical != lexical) { result.emplace_back(lexical, std::move(current)); current.clear(); }
    lexical = next_lexical; current.append(text, index, width); index += width;
  }
  if (!current.empty()) result.emplace_back(lexical, std::move(current));
  return result;
}
std::string case_marker(const std::string& token) {
  bool has_alpha = false, all_upper = true, first_upper = false;
  for (std::size_t i = 0; i < token.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(token[i]);
    if (!std::isalpha(ch) || ch >= 128) continue;
    has_alpha = true; if (std::islower(ch)) all_upper = false; if (i == 0 && std::isupper(ch)) first_upper = true;
  }
  if (!has_alpha) return "ma";       // unchanged / lower case
  if (all_upper) return "mi";         // upper case
  if (first_upper) return "me";       // title case
  return "ma";                        // lower case
}
std::string apply_case_marker(std::string value, const std::string& marker) {
  if (marker == "mi") { for (char& ch : value) if (static_cast<unsigned char>(ch) < 128) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch))); }
  else if (marker == "me" && !value.empty() && static_cast<unsigned char>(value.front()) < 128) value.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(value.front())));
  return value;
}
std::optional<std::string> unescape_text_token(const std::string& payload) {
  constexpr std::string_view prefix{"huhuhu"};
  if (!payload.starts_with(prefix) || (payload.size() - prefix.size()) % 6 != 0) return std::nullopt;
  std::string result;
  for (std::size_t i = prefix.size(); i < payload.size(); i += 6) {
    if (payload[i + 1] != 'a' || payload[i + 3] != 'a' || payload[i + 5] != 'a') return std::nullopt;
    const auto high = std::string_view{"mnlrptkbdgfshv"}.find(payload[i]);
    const auto middle = std::string_view{"mnlrptkbdgfshv"}.find(payload[i + 2]);
    const auto low = std::string_view{"mnlrptkbdgfshv"}.find(payload[i + 4]);
    if (high == std::string_view::npos || middle == std::string_view::npos || low == std::string_view::npos) return std::nullopt;
    result.push_back(static_cast<char>(high * kOnsets.size() * kOnsets.size() + middle * kOnsets.size() + low));
  }
  return result;
}

// The seed-language terms are deliberately small and reviewed.  Imported
// Wikidata labels have no machine-made German gloss: an untranslated label is
// surfaced as such to keep provenance and uncertainty visible.
const std::map<std::string, std::string>& german_terms() {
  static const std::map<std::string, std::string> terms{
      {"statement", "aussage"}, {"question", "frage"}, {"request", "bitte"},
      {"human", "mensch"}, {"machine", "maschine"}, {"child", "kind"}, {"researcher", "forscher"}, {"agent", "agent"},
      {"move", "bewegen"}, {"give", "geben"}, {"observe", "beobachten"}, {"build", "bauen"}, {"ask", "fragen"},
      {"stone", "stein"}, {"signal", "signal"}, {"map", "karte"}, {"water", "wasser"}, {"idea", "idee"},
      {"near", "nahe"}, {"before", "vor"}, {"because", "weil"}, {"if", "wenn"},
      {"certain", "sicher"}, {"possible", "möglich"}, {"required", "erforderlich"},
      {"unlikely", "unwahrscheinlich"}, {"likely", "wahrscheinlich"}, {"unknown", "unbekannt"},
      {"neutral", "neutral"}, {"joy", "freude"}, {"concern", "sorge"}, {"peer", "gleichrangig"}, {"ally", "verbündeter"}, {"observer", "beobachter"},
      {"inform", "informieren"}, {"assist", "helfen"}, {"coordinate", "koordinieren"}, {"direct", "direkt"}, {"reported", "berichtet"},
      {"literal", "wörtlich"}, {"metaphoric", "metaphorisch"}, {"forest", "wald"}, {"fire", "feuer"}, {"danger", "gefahr"},
      {"forest-fire", "waldbrand"}, {"rain", "regen"}, {"flood", "überschwemmung"}, {"tree", "baum"}, {"melody", "melodie"}};
  return terms;
}

std::string localize_label(const std::string& label, const std::string& language, bool& fallback) {
  if (language == "en") return label;
  const auto found = german_terms().find(canonical_concept_key(label));
  if (found != german_terms().end()) return found->second;
  fallback = true;
  return label;
}

std::optional<Meaning> analyse_meaning_sentence(const std::string& text) {
  const auto input = words(text);
  if (input.empty()) return std::nullopt;
  Meaning meaning{};
  bool actor{}, action{}, object{};
  const std::map<std::string, std::string> aliases{
      {"mensch", "human"}, {"human", "human"}, {"person", "human"},
      {"maschine", "machine"}, {"machine", "machine"}, {"kind", "child"}, {"child", "child"},
      {"forscher", "researcher"}, {"researcher", "researcher"}, {"agent", "agent"},
      {"bewegen", "move"}, {"bewegt", "move"}, {"move", "move"},
      {"geben", "give"}, {"gibt", "give"}, {"give", "give"},
      {"beobachten", "observe"}, {"beobachtet", "observe"}, {"observe", "observe"},
      {"bauen", "build"}, {"baut", "build"}, {"build", "build"},
      {"fragen", "ask"}, {"fragt", "ask"}, {"ask", "ask"},
      {"stein", "stone"}, {"stone", "stone"}, {"signal", "signal"},
      {"karte", "map"}, {"map", "map"}, {"wasser", "water"}, {"water", "water"},
      {"idee", "idea"}, {"idea", "idea"}};
  const std::map<std::string, std::string> relations{{"nahe", "near"}, {"near", "near"}, {"vor", "before"}, {"before", "before"}, {"weil", "because"}, {"because", "because"}, {"wenn", "if"}, {"if", "if"}};
  for (const auto& token : input) {
    if (const auto found = aliases.find(token); found != aliases.end()) {
      const std::string& value = found->second;
      if (value == "human" || value == "machine" || value == "child" || value == "researcher" || value == "agent") { meaning.actor = value; actor = true; }
      else if (value == "move" || value == "give" || value == "observe" || value == "build" || value == "ask") { meaning.action = value; action = true; }
      else { meaning.object = value; object = true; }
    }
    if (const auto found = relations.find(token); found != relations.end()) meaning.relation = found->second;
    if (token == "nicht" || token == "not") meaning.negated = true;
    if (token == "bitte" || token == "please") meaning.act = SpeechAct::request;
    if (token == "gestern" || token == "past") meaning.time = 1;
    if (token == "morgen" || token == "future") meaning.time = 2;
    if (token == "zwei" || token == "two") meaning.quantity = 2;
    if (token == "drei" || token == "three") meaning.quantity = 3;
    if (token == "möglich" || token == "possible") meaning.modality = "possible";
    if (token == "erforderlich" || token == "required") meaning.modality = "required";
    if (token == "unwahrscheinlich" || token == "unlikely") meaning.probability = "unlikely";
    if (token == "unbekannt" || token == "unknown") meaning.probability = "unknown";
    if (token == "freude" || token == "joy") meaning.emotion = "joy";
    if (token == "sorge" || token == "concern") meaning.emotion = "concern";
  }
  if (text.find('?') != std::string::npos) meaning.act = SpeechAct::question;
  if (!actor || !action || !object) return std::nullopt;
  return meaning;
}

std::string render_meaning(const Meaning& meaning, const std::string& language) {
  const bool german = language == "de";
  bool fallback{};
  const auto label = [&](const std::string& value) { return german ? localize_label(value, "de", fallback) : value; };
  const std::array<std::pair<int, const char*>, 3> times{{{0, german ? "Gegenwart" : "present"}, {1, german ? "Vergangenheit" : "past"}, {2, german ? "Zukunft" : "future"}}};
  const auto time = std::find_if(times.begin(), times.end(), [&meaning](const auto& item) { return item.first == meaning.time; });
  std::ostringstream out;
  out << (german ? "Bedeutung: " : "Meaning: ") << label(act_name(meaning.act)) << "; "
      << label(meaning.actor) << " / " << label(meaning.action) << " / " << label(meaning.object)
      << "\n" << (german ? "Relation: " : "Relation: ") << label(meaning.relation)
      << "; " << (german ? "Negation: " : "Negation: ") << (meaning.negated ? (german ? "ja" : "yes") : (german ? "nein" : "no"))
      << "; " << (german ? "Zeit: " : "Time: ") << (time == times.end() ? "?" : time->second)
      << "; " << (german ? "Menge: " : "Quantity: ") << meaning.quantity
      << "\n" << (german ? "Modalität: " : "Modality: ") << label(meaning.modality)
      << "; " << (german ? "Wahrscheinlichkeit: " : "Probability: ") << label(meaning.probability)
      << "; " << (german ? "Emotion: " : "Emotion: ") << label(meaning.emotion)
      << "; " << (german ? "Sozial: " : "Social: ") << label(meaning.social_relation)
      << "; " << (german ? "Ziel: " : "Goal: ") << label(meaning.goal)
      << "; " << (german ? "Aussageebene: " : "Proposition: ") << label(meaning.proposition_level)
      << "; " << (german ? "Abstraktion: " : "Abstraction: ") << label(meaning.abstraction);
  return out.str();
}

std::string stable_fingerprint(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("cannot read concept import: " + path.string());
  std::uint64_t hash = 1469598103934665603ULL;
  char buffer[8192];
  std::uint64_t length{};
  while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0) {
    const auto count = in.gcount(); length += static_cast<std::uint64_t>(count);
    for (std::streamsize i = 0; i < count; ++i) { hash ^= static_cast<unsigned char>(buffer[i]); hash *= 1099511628211ULL; }
  }
  std::ostringstream out; out << std::hex << hash << '-' << std::dec << length;
  return out.str();
}

std::string generated_concept_id(std::size_t serial) {
  std::ostringstream out; out << "CONCEPT_" << std::setw(8) << std::setfill('0') << serial;
  return out.str();
}

std::vector<std::string> split_tsv(const std::string& line) {
  std::vector<std::string> fields;
  std::istringstream stream(line);
  for (std::string field; std::getline(stream, field, '\t');) fields.push_back(trim(std::move(field)));
  return fields;
}

bool validate_compositional_form(const ConceptEntry& entry,
                                 const std::map<std::string, std::string>& receiver) {
  // The receiver reconstructs labels from transmitted forms, not concept IDs.
  if (entry.derivation == "root") return receiver.contains(entry.form) && receiver.at(entry.form) == entry.label;
  std::istringstream parts(entry.form);
  std::string part;
  std::vector<std::string> recovered;
  while (std::getline(parts, part, '-')) {
    const auto known = receiver.find(part);
    if (known == receiver.end()) return false;
    recovered.push_back(known->second);
  }
  return join(recovered, "-") == entry.label;
}

bool externally_qualified(const ConceptEntry& entry) {
  // A row is qualified only when every externally auditable anchor is still
  // present.  In particular, an arbitrary generated stress label cannot pass
  // merely by being stored in the same graph.
  return entry.evidence_class == "external-attested" && !entry.external_id.empty() &&
         !entry.label.empty() && !entry.semantics.empty() && !entry.form.empty() &&
         !entry.source.empty() && !entry.provenance.empty() && !entry.source_revision.empty() &&
         entry.provenance.find("CC0") != std::string::npos;
}

Candidate make_candidate(std::uint64_t id, std::uint64_t parent, std::mt19937_64& rng) {
  Candidate candidate{};
  candidate.id = id;
  candidate.parent = parent;
  std::set<std::string> used;
  for (const auto& a : kActors) { const Meaning m{SpeechAct::statement, a, "move", "stone", "near", false, 0, 1}; candidate.lexicon.emplace("actor:" + std::string(a), new_word(rng, used)); }
  for (const auto& a : kActions) candidate.lexicon.emplace("action:" + std::string(a), new_word(rng, used));
  for (const auto& a : kObjects) candidate.lexicon.emplace("object:" + std::string(a), new_word(rng, used));
  for (const auto& a : kRelations) candidate.lexicon.emplace("relation:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"statement", "question", "request"}) candidate.lexicon.emplace("act:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"0", "1"}) candidate.lexicon.emplace("neg:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"0", "1", "2"}) candidate.lexicon.emplace("time:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"1", "2", "3"}) candidate.lexicon.emplace("qty:" + std::string(a), new_word(rng, used));
  for (const auto& a : kModalities) candidate.lexicon.emplace("modality:" + std::string(a), new_word(rng, used));
  for (const auto& a : kProbabilities) candidate.lexicon.emplace("probability:" + std::string(a), new_word(rng, used));
  for (const auto& a : kEmotions) candidate.lexicon.emplace("emotion:" + std::string(a), new_word(rng, used));
  for (const auto& a : kSocialRelations) candidate.lexicon.emplace("social:" + std::string(a), new_word(rng, used));
  for (const auto& a : kGoals) candidate.lexicon.emplace("goal:" + std::string(a), new_word(rng, used));
  for (const auto& a : kPropositionLevels) candidate.lexicon.emplace("proposition:" + std::string(a), new_word(rng, used));
  for (const auto& a : kAbstractions) candidate.lexicon.emplace("abstraction:" + std::string(a), new_word(rng, used));
  ensure_morphology(candidate, rng);
  candidate.order = default_order();
  return candidate;
}
Candidate mutate(const Candidate& source, std::uint64_t id, std::mt19937_64& rng) {
  Candidate out = source; out.id = id; out.parent = source.id; out.metrics = {};
  if (out.order.empty()) out.order = default_order();
  std::bernoulli_distribution mutate_grammar(0.35);
  if (mutate_grammar(rng)) {
    std::uniform_int_distribution<std::size_t> select(0, out.order.size() - 1);
    const auto first = select(rng); auto second = select(rng);
    while (second == first) second = select(rng);
    std::swap(out.order[first], out.order[second]);
    return out;
  }
  ensure_morphology(out, rng);
  std::set<std::string> used; for (const auto& [_, word] : out.lexicon) used.insert(word); for (const auto& [_, word] : out.morphology) used.insert(word);
  std::bernoulli_distribution mutate_morphology(0.20);
  if (mutate_morphology(rng)) {
    std::uniform_int_distribution<std::size_t> select(0, out.morphology.size() - 1);
    auto it = out.morphology.begin(); std::advance(it, static_cast<long>(select(rng))); used.erase(it->second); it->second = new_word(rng, used);
  } else {
    std::uniform_int_distribution<std::size_t> select(0, out.lexicon.size() - 1);
    auto it = out.lexicon.begin(); std::advance(it, static_cast<long>(select(rng))); used.erase(it->second); it->second = new_word(rng, used);
  }
  return out;
}
std::string encode(const Candidate& c, const Meaning& m) {
  std::ostringstream out;
  for (const auto& atom : ordered_atoms(c, m)) { if (out.tellp() > 0) out << ' '; out << c.lexicon.at(atom); }
  return out.str();
}
bool decode(const std::map<std::string, std::string>& private_knowledge, const std::string& message, Meaning& out) {
  std::istringstream in(message); std::string word; std::map<std::string, std::string> values;
  while (in >> word) { const auto it = private_knowledge.find(word); if (it == private_knowledge.end()) return false; const auto pos = it->second.find(':'); values[it->second.substr(0, pos)] = it->second.substr(pos + 1); }
  if (values.size() != 15) return false;
  out = {parse_act(values["act"]), values["actor"], values["action"], values["object"], values["relation"], values["neg"] == "1", std::stoi(values["time"]), std::stoi(values["qty"]), values["modality"], values["probability"], values["emotion"], values["social"], values["goal"], values["proposition"], values["abstraction"]};
  return true;
}

std::string productive_root_form(const Candidate& language, const std::string& root) {
  // Core meanings preserve their evolved lexical roots. Additional verbs are
  // regular, reproducible derivations: learning the base form is enough for
  // a receiver to recognize all of its marked forms.
  const std::map<std::string, std::string> core{{"gehen", "action:move"}, {"bauen", "action:build"}, {"fragen", "action:ask"}};
  if (const auto found = core.find(root); found != core.end()) return language.lexicon.at(found->second);
  std::uint64_t hash = 1469598103934665603ULL;
  const std::string key = language.lexicon.at("action:move") + ":" + root;
  for (const unsigned char ch : key) { hash ^= ch; hash *= 1099511628211ULL; }
  std::string form;
  for (int i = 0; i < 4; ++i) {
    form += kOnsets[hash % kOnsets.size()]; hash /= kOnsets.size();
    form += kVowels[hash % kVowels.size()]; hash /= kVowels.size();
  }
  return form;
}

std::string productive_grammar_test(const Candidate& language) {
  // Receiver training deliberately excludes the held-out `lernen;pl;past`.
  // It is given only the base form of lernen, markers from other verbs, and
  // must recover the unseen combination without an answer-side lookup.
  const std::map<std::string, std::string> roots{{"gehen", productive_root_form(language, "gehen")}, {"bauen", productive_root_form(language, "bauen")}, {"fragen", productive_root_form(language, "fragen")}, {"lernen", productive_root_form(language, "lernen")}};
  const auto& markers = language.morphology;
  const auto encode = [&roots, &markers](const std::string& root, const std::string& number, const std::string& time) { return roots.at(root) + "-" + markers.at("person:3") + markers.at("number:" + number) + markers.at("time:" + time); };
  const std::string seen_singular = encode("gehen", "sg", "present");
  const std::string held_out_plural = encode("lernen", "pl", "past");
  struct MorphologyReceiver {
    std::map<std::string, std::string> roots;
    std::map<std::string, std::string> markers;
  } receiver;
  // Feedback from separate, already observed utterances.  The receiver never
  // gets the lernen/pl/past combination or the sender's maps themselves.
  for (const auto& root : {"gehen", "bauen", "fragen", "lernen"}) receiver.roots.emplace(roots.at(root), root);
  receiver.markers.emplace(markers.at("person:3"), "person:3");
  receiver.markers.emplace(markers.at("number:sg"), "number:sg");
  receiver.markers.emplace(markers.at("number:pl"), "number:pl");
  receiver.markers.emplace(markers.at("time:present"), "time:present");
  receiver.markers.emplace(markers.at("time:past"), "time:past");
  const auto decode = [&receiver](const std::string& form) {
    const auto split = form.find('-'); if (split == std::string::npos) return std::string{};
    const auto root_known = receiver.roots.find(form.substr(0, split));
    if (root_known == receiver.roots.end()) return std::string{};
    const std::string& root = root_known->second;
    const std::string suffix = form.substr(split + 1);
    for (const auto& [person_surface, person] : receiver.markers) for (const auto& [number_surface, number] : receiver.markers) for (const auto& [time_surface, time] : receiver.markers)
      if (person == "person:3" && number.starts_with("number:") && time.starts_with("time:") && suffix == person_surface + number_surface + time_surface)
        return root + ";" + number.substr(7) + ";" + time.substr(5);
    return std::string{};
  };
  const std::map<std::string, std::string> german_family{{"gehen", "gehen;infinitive"}, {"geht", "gehen;3;sg;present"}, {"ging", "gehen;3;sg;past"}, {"gingen", "gehen;3;pl;past"}, {"gegangen", "gehen;participle;past"}, {"lernen", "lernen;infinitive"}, {"lernt", "lernen;3;sg;present"}, {"lernten", "lernen;3;pl;past"}};
  const bool family_ok = german_family.at("geht").starts_with("gehen;") && german_family.at("ging").starts_with("gehen;") && german_family.at("gegangen").starts_with("gehen;");
  const bool held_out_ok = decode(held_out_plural) == "lernen;pl;past";
  std::ostringstream out;
  out << "Productive morphology lab\nGerman family analysis: gehen / geht / ging / gegangen -> root=gehen\nfamily recognition: " << (family_ok ? "passed" : "failed") << "\nroot gehen: " << roots.at("gehen") << "\nseen singular present: " << seen_singular << " = gehen;sg;present\nnew verb base supplied to receiver: lernen = " << roots.at("lernen") << "\nheld-out new-verb plural past: " << held_out_plural << "\nreceiver training: base forms plus markers from other utterances; held-out combination absent\nindependent receiver reconstruction: " << decode(held_out_plural) << "\nregular-marker validation: " << (held_out_ok ? "passed" : "failed") << "\nmarkers: person-3=" << markers.at("person:3") << ", singular=" << markers.at("number:sg") << ", plural=" << markers.at("number:pl") << ", present=" << markers.at("time:present") << ", past=" << markers.at("time:past");
  return out.str();
}

struct GermanVerbFeatures { std::string root; std::string number; std::string time; };
std::optional<GermanVerbFeatures> analyse_german_verb(const std::string& token, const std::string& previous = {}) {
  static const std::map<std::string, GermanVerbFeatures> forms{
      {"gehen", {"gehen", "", "present"}}, {"geht", {"gehen", "sg", "present"}}, {"ging", {"gehen", "sg", "past"}}, {"gingen", {"gehen", "pl", "past"}}, {"gegangen", {"gehen", "sg", "past"}},
      {"bauen", {"bauen", "", "present"}}, {"baut", {"bauen", "sg", "present"}}, {"baute", {"bauen", "sg", "past"}}, {"bauten", {"bauen", "pl", "past"}}, {"gebaut", {"bauen", "sg", "past"}},
      {"fragen", {"fragen", "", "present"}}, {"fragt", {"fragen", "sg", "present"}}, {"fragte", {"fragen", "sg", "past"}}, {"fragten", {"fragen", "pl", "past"}}, {"gefragt", {"fragen", "sg", "past"}}};
  const std::string word = canonical_concept_key(token);
  auto found = forms.find(word);
  GermanVerbFeatures result;
  if (found != forms.end()) result = found->second;
  else if (word.size() > 4 && word.ends_with("ten")) result = {word.substr(0, word.size() - 3) + "en", "pl", "past"};
  else if (word.size() > 3 && word.ends_with("te")) result = {word.substr(0, word.size() - 2) + "en", "sg", "past"};
  else if (word.size() > 4 && word.starts_with("ge") && word.ends_with("t")) result = {word.substr(2, word.size() - 3) + "en", "sg", "past"};
  else if (word.size() > 3 && word.ends_with("en")) result = {word, "", "present"};
  else if (word.size() > 3 && word.ends_with("t")) result = {word.substr(0, word.size() - 1) + "en", "sg", "present"};
  else return std::nullopt;
  if (result.number.empty() && (canonical_concept_key(previous) == "sie" || canonical_concept_key(previous) == "wir")) result.number = "pl";
  return result;
}

std::string productive_verb_form(const Candidate& language, const GermanVerbFeatures& features) {
  const std::string root = productive_root_form(language, features.root);
  if (features.number.empty()) return root;
  return root + "-" + language.morphology.at("person:3") + language.morphology.at("number:" + features.number) + language.morphology.at("time:" + features.time);
}
void train_population(const Candidate& language, std::vector<AgentState>& agents, std::mt19937_64& rng, int rounds) {
  if (agents.empty()) return;
  for (int step = 0; step < rounds; ++step) {
    const Meaning expected = random_meaning(rng, false);
    const std::string message = encode(language, expected);
    const auto semantic_atoms = ordered_atoms(language, expected);
    std::istringstream tokens(message);
    std::vector<std::string> words;
    for (std::string word; tokens >> word;) words.push_back(std::move(word));
    for (auto& agent : agents) {
      Meaning decoded;
      if (decode(agent.private_reverse_lexicon, message, decoded) && decoded == expected) ++agent.successful_interpretations;
      else ++agent.failed_interpretations;
      // The feedback is delivered only after the receiver has made its attempt.
      for (std::size_t index = 0; index < words.size(); ++index) agent.private_reverse_lexicon[words[index]] = semantic_atoms.at(index);
    }
  }
}
Metrics evaluate(const Candidate& candidate, std::mt19937_64& rng, int train, int evaluation) {
  // Each receiver owns a reverse lexicon. No decoder receives the expected Meaning.
  std::map<std::string, std::string> receiver;
  for (int i = 0; i < train; ++i) {
    const Meaning m = random_meaning(rng, false); const auto message = encode(candidate, m);
    std::istringstream tokens(message); std::string word; const auto a = atoms(m); std::size_t index = 0;
    while (tokens >> word) receiver[word] = a.at(index++); // explicit feedback after a completed training trial
  }
  int exact = 0; int recovered_fields = 0; int all_fields = 0; int robust_fields = 0; int robust_total_fields = 0; std::size_t bytes = 0;
  for (int i = 0; i < evaluation; ++i) {
    const Meaning expected = random_meaning(rng, true); const std::string message = encode(candidate, expected); bytes += message.size();
    Meaning decoded; const bool valid = decode(receiver, message, decoded); if (valid && decoded == expected) ++exact;
    const auto target = atoms(expected); const auto result = valid ? atoms(decoded) : std::vector<std::string>{};
    for (std::size_t p = 0; p < target.size(); ++p) { ++all_fields; if (p < result.size() && target[p] == result[p]) ++recovered_fields; }
    // Controlled transmission error: substitute exactly one actor token with a
    // different valid actor word. This tests semantic degradation rather than
    // treating a malformed byte sequence as an uninformative decoder failure.
    std::istringstream message_stream(message);
    std::vector<std::string> noisy_words;
    for (std::string word; message_stream >> word;) noisy_words.push_back(std::move(word));
    noisy_words.at(1) = candidate.lexicon.at("actor:" + std::string(expected.actor == "human" ? "machine" : "human"));
    std::ostringstream noisy_message;
    for (std::size_t p = 0; p < noisy_words.size(); ++p) { if (p > 0) noisy_message << ' '; noisy_message << noisy_words[p]; }
    Meaning noisy_decoded;
    const bool noisy_valid = decode(receiver, noisy_message.str(), noisy_decoded);
    const auto noisy_result = noisy_valid ? atoms(noisy_decoded) : std::vector<std::string>{};
    for (std::size_t p = 0; p < target.size(); ++p) { ++robust_total_fields; if (p < noisy_result.size() && target[p] == noisy_result[p]) ++robust_fields; }
  }
  // Morphological markers participate in selection just like roots: compact,
  // pronounceable reusable markers lower the cost of unseen combinations.
  double pronounceability = 0.0; for (const auto& [_, word] : candidate.lexicon) pronounceability += word_quality(word);
  std::size_t morphology_bytes{};
  for (const auto& [_, word] : candidate.morphology) { pronounceability += word_quality(word); morphology_bytes += word.size(); }
  pronounceability /= static_cast<double>(candidate.lexicon.size() + candidate.morphology.size());
  Metrics m; m.communication_success = static_cast<double>(exact) / evaluation; m.semantic_accuracy = static_cast<double>(recovered_fields) / all_fields;
  // One representative unseen verb combination charges the reusable marker
  // payload once per evaluation batch, instead of treating morphology as free.
  m.cost = static_cast<double>(bytes + morphology_bytes) / evaluation; m.pronounceability = pronounceability;
  const auto& order = candidate.order.empty() ? default_order() : candidate.order;
  int inversions{};
  for (std::size_t i = 0; i < order.size(); ++i) for (std::size_t j = i + 1; j < order.size(); ++j) {
    const auto left = std::find(default_order().begin(), default_order().end(), order[i]) - default_order().begin();
    const auto right = std::find(default_order().begin(), default_order().end(), order[j]) - default_order().begin();
    if (left > right) ++inversions;
  }
  const double max_inversions = static_cast<double>(order.size() * (order.size() - 1) / 2);
  m.grammar_complexity = 1.0 + static_cast<double>(inversions) / max_inversions;
  m.robustness = static_cast<double>(robust_fields) / robust_total_fields; m.expressiveness = 1.0; return m;
}
void rank_candidates(std::vector<Candidate>& candidates) {
  std::vector<double> benefit;
  std::vector<double> cost;
  benefit.reserve(candidates.size());
  cost.reserve(candidates.size());
  for (const auto& candidate : candidates) {
    const auto& m = candidate.metrics;
    benefit.push_back(m.communication_success * 45.0 + m.semantic_accuracy * 25.0 + m.pronounceability * 15.0 + m.robustness * 10.0 + m.expressiveness * 5.0 - m.grammar_complexity * 4.0);
    cost.push_back(m.cost);
  }
  const auto batch = compute_reward_batch(benefit, cost, 0.08);
  for (std::size_t i = 0; i < candidates.size(); ++i) candidates[i].selection_score = batch.values[i];
}
bool dominates(const Metrics& a, const Metrics& b) {
  const std::array<double, 7> av{a.communication_success, a.semantic_accuracy, -a.cost, a.pronounceability, -a.grammar_complexity, a.robustness, a.expressiveness};
  const std::array<double, 7> bv{b.communication_success, b.semantic_accuracy, -b.cost, b.pronounceability, -b.grammar_complexity, b.robustness, b.expressiveness};
  bool strict = false; for (std::size_t i = 0; i < av.size(); ++i) { if (av[i] < bv[i]) return false; strict = strict || av[i] > bv[i]; } return strict;
}
CandidateSummary summary(const Candidate& c) { return {c.id, c.parent, c.metrics, encode(c, Meaning{SpeechAct::statement, "human", "observe", "idea", "because", false, 1, 1})}; }
std::string canonical_answer(const std::string& input) {
  std::string out;
  bool previous_space = true;
  for (const unsigned char character : input) {
    if (std::isspace(character)) { if (!previous_space) out += ' '; previous_space = true; }
    else { out += static_cast<char>(std::tolower(character)); previous_space = false; }
  }
  if (!out.empty() && out.back() == ' ') out.pop_back();
  return out;
}
std::string grammar_description(const Candidate& candidate) {
  const auto& order = candidate.order.empty() ? default_order() : candidate.order;
  std::ostringstream out;
  for (const auto& atom : order) {
    if (out.tellp() > 0) out << ' ';
    const auto separator = atom.find(':');
    out << atom.substr(0, separator);
  }
  return out.str();
}
std::uint64_t unix_millis() { return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()); }
}  // namespace

std::string Meaning::key() const { std::ostringstream out; for (const auto& a : atoms(*this)) out << a << '|'; return out.str(); }
double Metrics::score() const { return communication_success * 45 + semantic_accuracy * 25 + pronounceability * 15 + robustness * 10 + expressiveness * 5 - grammar_complexity * 4 - cost * 0.08; }

struct ResearchEngine::Impl {
  RunConfig config; std::mt19937_64 rng; int generation{}; std::uint64_t next_id{1}; std::vector<Candidate> frontier; std::vector<CandidateSummary> history; std::vector<AgentState> agents; std::vector<ConceptEntry> concepts;
  // V12 keeps insertion order for reproducible exports while all lookups use
  // these indexes.  This removes the former O(n²) import behaviour.
  std::unordered_map<std::string, std::size_t> by_identity;
  std::unordered_map<std::string, std::size_t> by_label;
  std::unordered_set<std::string> used_forms;
  std::vector<GrammarLexeme> grammar_lexemes;
  std::size_t next_concept_serial{1};
  ImportCheckpoint import_checkpoint;
  ImportCheckpoint german_word_list_checkpoint;
  std::uint64_t human_attempts{}; std::uint64_t human_correct{}; std::uint64_t human_total_millis{}; std::uint64_t human_production_attempts{}; std::uint64_t human_production_correct{}; std::uint64_t human_interpretation_attempts{}; std::uint64_t human_interpretation_correct{};
  explicit Impl(RunConfig c) : config(c), rng(c.seed) { if (c.population < 2 || c.generations < 1) throw std::invalid_argument("population must be >= 2 and generations >= 1"); }
  void ensure_concept_lexicon();
  void rebuild_concept_indexes();
  ConceptEntry* find_concept(const std::string& identity_or_label);
  void add_concept(ConceptEntry entry);
};

void enrich_neu_lunari_lexemes(std::vector<GrammarLexeme>& entries) {
  for (auto& entry : entries) {
    if (entry.concept_id.starts_with("grammar:pronoun:")) { entry.part_of_speech = "pronoun"; entry.valency = "argument"; entry.scope = "noun phrase"; }
    else if (entry.concept_id.starts_with("grammar:relation:")) { entry.part_of_speech = "relation word"; entry.valency = "relation + complement"; entry.scope = "verb phrase or noun phrase"; }
    else if (entry.concept_id.starts_with("grammar:modality:")) { entry.part_of_speech = "modal particle"; entry.valency = "predicate"; entry.scope = "verb phrase"; }
    else if (entry.concept_id.starts_with("grammar:question:")) { entry.part_of_speech = "interrogative"; entry.valency = "argument position"; entry.scope = "clause"; }
    else if (entry.concept_id.starts_with("grammar:connective:") || entry.concept_id.starts_with("grammar:embedding:")) { entry.part_of_speech = "clause linker"; entry.valency = "two clauses or embedded clause"; entry.scope = "clause"; }
    else if (entry.concept_id.starts_with("grammar:degree:")) { entry.part_of_speech = "degree particle"; entry.valency = "adjective"; entry.scope = "noun phrase or predicate"; }
    else if (entry.concept_id.starts_with("grammar:act:") || entry.concept_id.starts_with("grammar:negation:") || entry.concept_id.starts_with("grammar:aspect:") || entry.concept_id.starts_with("grammar:time:") || entry.concept_id.starts_with("grammar:stance:") || entry.concept_id.starts_with("grammar:probability:")) { entry.part_of_speech = "grammatical particle"; entry.valency = "clause"; entry.scope = "clause"; }
    else if (entry.concept_id.starts_with("grammar:determiner:")) { entry.part_of_speech = "determiner"; entry.valency = "noun"; entry.scope = "noun phrase"; }
    else if (entry.concept_id.starts_with("grammar:number:")) { entry.part_of_speech = "quantity particle"; entry.valency = "noun"; entry.scope = "noun phrase"; }
    else { entry.part_of_speech = "lexeme"; entry.valency = "unspecified"; entry.scope = "lexical"; }
    if (entry.meaning.empty()) { const auto pos = entry.concept_id.find_last_of(':'); entry.meaning = pos == std::string::npos ? entry.concept_id : entry.concept_id.substr(pos + 1); }
  }
}

void ensure_neu_lunari_lexemes(std::vector<GrammarLexeme>& entries) {
  const auto add = [&](const char* id, const char* surface, const char* note, const char* historical = "") {
    if (std::any_of(entries.begin(), entries.end(), [id](const GrammarLexeme& entry) { return entry.concept_id == id; })) return;
    entries.push_back({id, historical, surface, "neu-lunari-v1-proposed", "proposed", "neu-lunari-v1-grammar", note, "", "", "", ""});
  };
  add("grammar:pronoun:first-singular", "mer", "first-person singular; collision audit clear", "memekumo");
  add("grammar:pronoun:second-singular", "gav", "second-person singular; collision audit clear", "gavogedu");
  add("grammar:pronoun:third-singular", "se", "gender-neutral third-person singular"); add("grammar:pronoun:third-plural", "sel", "third-person plural"); add("grammar:pronoun:second-plural", "gal", "second-person plural"); add("grammar:pronoun:first-plural-inclusive", "rin", "inclusive we"); add("grammar:pronoun:first-plural-exclusive", "lan", "exclusive we"); add("grammar:pronoun:reflexive", "sir", "reflexive reference"); add("grammar:pronoun:reciprocal", "san", "reciprocal reference");
  add("grammar:act:question", "ke", "clause-final question"); add("grammar:act:request", "va", "request"); add("grammar:act:prohibition", "vas", "prohibition"); add("grammar:act:answer-yes", "ya", "affirmative short answer"); add("grammar:act:answer-no", "na", "negative short answer");
  add("grammar:time:past", "ren", "past"); add("grammar:time:future", "sil", "future"); add("grammar:aspect:progressive", "lir", "progressive"); add("grammar:aspect:completed", "sen", "completed"); add("grammar:aspect:habitual", "tan", "habitual");
  add("grammar:negation:predicate", "nus", "predicate negation"); add("grammar:negation:statement", "nor", "statement negation"); add("grammar:voice:passive", "vel", "passive"); add("grammar:determiner:definite", "tel", "definite"); add("grammar:determiner:indefinite", "sar", "indefinite"); add("grammar:number:plural", "lin", "plural");
  add("grammar:modality:ability", "kan", "ability"); add("grammar:modality:possibility", "ves", "possibility"); add("grammar:modality:permission", "dar", "permission"); add("grammar:modality:obligation", "mur", "obligation"); add("grammar:modality:recommendation", "res", "recommendation"); add("grammar:modality:intent", "vin", "intent"); add("grammar:modality:wish", "lum", "wish");
  add("grammar:relation:location", "nel", "location"); add("grammar:relation:destination", "tal", "destination"); add("grammar:relation:source", "sor", "source"); add("grammar:relation:recipient", "rem", "recipient"); add("grammar:relation:instrument", "ket", "instrument"); add("grammar:relation:companion", "val", "companion"); add("grammar:relation:topic", "tem", "topic"); add("grammar:relation:purpose", "sur", "purpose"); add("grammar:relation:possession", "bel", "possession"); add("grammar:relation:part", "pel", "part/whole"); add("grammar:relation:content", "mir", "content"); add("grammar:relation:material", "dam", "material");
  add("grammar:question:person", "man", "person question"); add("grammar:question:thing", "min", "thing/content question"); add("grammar:question:place", "lon", "place question"); add("grammar:question:time", "tir", "time question"); add("grammar:question:reason", "kar", "reason question"); add("grammar:question:manner", "sev", "manner question"); add("grammar:question:amount", "num", "amount question"); add("grammar:question:choice", "var", "choice question");
  add("grammar:negation:subject-focus", "sam", "subject focus negation"); add("grammar:negation:object-focus", "som", "object focus negation"); add("grammar:negation:modality-focus", "nam", "modality focus negation");
  add("grammar:probability:probable", "pur", "probability"); add("grammar:probability:uncertain", "sul", "uncertainty"); add("grammar:stance:report", "rav", "reported statement"); add("grammar:stance:hypothesis", "hes", "hypothesis"); add("grammar:stance:fiction", "fik", "fictional frame");
  add("grammar:scope:emotion", "fel", "emotion scope marker"); add("grammar:scope:social", "saf", "social-relation scope marker"); add("grammar:scope:goal", "gol", "goal scope marker");
  add("grammar:connective:and", "ran", "coordination"); add("grammar:connective:or", "lor", "alternative"); add("grammar:connective:but", "bas", "contrast"); add("grammar:connective:cause", "kos", "cause"); add("grammar:connective:condition", "kim", "condition"); add("grammar:connective:concession", "lus", "concession"); add("grammar:connective:purpose", "suv", "clause purpose"); add("grammar:connective:temporal", "tim", "temporal relation");
  add("grammar:embedding:content-open", "ki", "content opening marker"); add("grammar:embedding:content-close", "ku", "content closing marker"); add("grammar:embedding:relative-open", "ril", "relative opening marker"); add("grammar:embedding:relative-close", "rul", "relative closing marker"); add("grammar:embedding:quote-open", "qi", "quote opening marker"); add("grammar:embedding:quote-close", "qu", "quote closing marker");
  add("grammar:number:about", "mar", "approximate quantity"); add("grammar:number:atleast", "las", "minimum quantity"); add("grammar:number:atmost", "sav", "maximum quantity"); add("grammar:number:fraction", "fal", "fraction"); add("grammar:number:ordinal", "tor", "ordinal quantity"); add("grammar:number:measure", "mun", "measure marker");
  add("grammar:degree:very", "daz", "intensifier"); add("grammar:degree:more", "mor", "comparative more"); add("grammar:degree:less", "les", "comparative less"); add("grammar:degree:equal", "sim", "equality comparison"); add("grammar:degree:most", "mas", "superlative"); add("grammar:degree:comparison", "kom", "comparison standard"); add("grammar:copula:state", "gul", "state copula");
  add("lexeme:adjective:good", "vaske", "accented display variant: vaské; collision audit clear", "vipakevo");
  enrich_neu_lunari_lexemes(entries);
}

void ResearchEngine::Impl::rebuild_concept_indexes() {
  by_identity.clear(); by_label.clear(); used_forms.clear(); next_concept_serial = 1;
  for (std::size_t i = 0; i < concepts.size(); ++i) {
    const auto& entry = concepts[i];
    by_identity.emplace(entry.external_id.empty() ? entry.id : entry.external_id, i);
    by_identity.emplace(entry.id, i);
    by_label.emplace(canonical_concept_key(entry.label), i);
    used_forms.emplace(entry.form);
    const auto digits = entry.id.find_last_not_of("0123456789");
    if (digits != std::string::npos && digits + 1 < entry.id.size()) {
      try { next_concept_serial = std::max(next_concept_serial, static_cast<std::size_t>(std::stoull(entry.id.substr(digits + 1)) + 1)); } catch (...) { }
    }
  }
  if (!frontier.empty()) for (const auto& [_, form] : frontier.front().lexicon) used_forms.emplace(form);
}
ConceptEntry* ResearchEngine::Impl::find_concept(const std::string& identity_or_label) {
  const auto identity = by_identity.find(identity_or_label);
  if (identity != by_identity.end()) return &concepts[identity->second];
  const auto label = by_label.find(canonical_concept_key(identity_or_label));
  return label == by_label.end() ? nullptr : &concepts[label->second];
}
void ResearchEngine::Impl::add_concept(ConceptEntry entry) {
  const auto position = concepts.size();
  if (entry.id.empty()) entry.id = generated_concept_id(next_concept_serial++);
  else {
    const auto digits = entry.id.find_last_not_of("0123456789");
    if (digits != std::string::npos && digits + 1 < entry.id.size()) {
      try { next_concept_serial = std::max(next_concept_serial, static_cast<std::size_t>(std::stoull(entry.id.substr(digits + 1)) + 1)); } catch (...) { }
    }
  }
  if (entry.language_status.empty()) entry.language_status = "pending";
  if (entry.evidence_class.empty()) entry.evidence_class = "agent-hypothesis";
  used_forms.emplace(entry.form);
  by_identity.emplace(entry.id, position);
  if (!entry.external_id.empty()) by_identity.emplace(entry.external_id, position);
  by_label.emplace(canonical_concept_key(entry.label), position);
  concepts.push_back(std::move(entry));
}

void ResearchEngine::Impl::ensure_concept_lexicon() {
  if (!concepts.empty()) return;
  if (!config.seed_concept_lexicon) return;
  rebuild_concept_indexes();
  std::set<std::string> used;
  for (const auto& [_, word] : frontier.front().lexicon) used.insert(word);
  const auto root = [this, &used](const char* id, const char* label, const char* part, const char* semantics, const char* relations) {
    ConceptEntry entry{}; entry.id = id; entry.label = label; entry.part_of_speech = part;
    entry.form = new_word(rng, used);
    entry.derivation = "root"; entry.source = "seeded-human-knowledge"; entry.provenance = "local-curated-starter-set";
    entry.source_revision = "local-v1"; entry.evidence_class = "agent-hypothesis"; entry.language_status = "realised";
    entry.semantics = semantics; entry.relations = relations; entry.introduced_generation = generation;
    add_concept(std::move(entry));
  };
  // A compact, inspectable initial knowledge basis. It is local curated seed
  // data, not a copy of a third-party dictionary; future imports retain source.
  root("CONCEPT_00001", "tree", "noun", "living;plant;woody", "part-of:forest;produces:wood,fruit;casts:shadow");
  root("CONCEPT_00002", "forest", "noun", "place;many-trees", "contains:tree;can-contain:fire");
  root("CONCEPT_00003", "fire", "noun", "event;heat;combustion", "can-threaten:forest;causes:danger");
  root("CONCEPT_00004", "danger", "noun", "abstract;risk", "can-describe:fire");
  root("CONCEPT_00005", "wood", "noun", "material;plant-derived", "derived-from:tree");
  root("CONCEPT_00006", "shadow", "noun", "phenomenon;light-blocked", "caused-by:tree");
  root("CONCEPT_00007", "fruit", "noun", "plant-product;edible", "produced-by:tree");
  const auto form_for = [this](const char* label) -> std::string { const auto* entry = find_concept(label); if (entry) return entry->form; throw std::logic_error("missing concept root"); };
  const auto compound = [this, &form_for](const char* id, const char* label, const char* semantics, const char* relations, const std::vector<std::string>& family) {
    ConceptEntry entry{}; entry.id = id; entry.label = label; entry.part_of_speech = "noun";
    entry.derivation = "compound"; entry.source = "simulated-experience"; entry.provenance = "derived-from-persisted-concept-relations";
    entry.source_revision = "local-v1"; entry.evidence_class = "simulated-relation"; entry.language_status = "realised";
    entry.semantics = semantics; entry.relations = relations; entry.introduced_generation = generation;
    std::vector<std::string> forms;
    for (const auto& member : family) forms.push_back(form_for(member.c_str()));
    entry.form = join(forms, "-");
    add_concept(std::move(entry));
  };
  // These are discovered in the simulated world through relations above, and
  // preserve their morphological family instead of receiving random tokens.
  compound("CONCEPT_00008", "forest-fire", "event;forest;combustion", "is-a:fire;occurs-in:forest", {"forest", "fire"});
  compound("CONCEPT_00009", "forest-fire-danger", "abstract;risk;forest-fire", "describes:forest-fire", {"forest", "fire", "danger"});

  std::map<std::string, std::string> receiver;
  for (const auto& entry : concepts) if (entry.derivation == "root") receiver.emplace(entry.form, entry.label);
  for (auto& entry : concepts) {
    // Multiple receivers must reconstruct a message from learned forms. No
    // comparison of opaque CONCEPT_* identifiers participates in this check.
    entry.validation_attempts = 6;
    for (int trial = 0; trial < 6; ++trial) if (validate_compositional_form(entry, receiver)) ++entry.validation_successes;
    entry.validation_failures = entry.validation_attempts - entry.validation_successes;
    entry.language_status = entry.validation_successes == entry.validation_attempts ? "communicatively-validated" : "realised";
  }
}

ResearchEngine::ResearchEngine(RunConfig config) : impl_(new Impl(config)) {}
ResearchEngine::~ResearchEngine() { delete impl_; }

void ResearchEngine::run() {
  if (impl_->frontier.empty()) {
    for (int i = 0; i < impl_->config.population; ++i) impl_->frontier.push_back(make_candidate(impl_->next_id++, 0, impl_->rng));
    for (int i = 0; i < impl_->config.population; ++i) impl_->agents.push_back(AgentState{static_cast<std::uint64_t>(i + 1), {}, 0, 0});
  }
  if (impl_->agents.empty()) for (int i = 0; i < impl_->config.population; ++i) impl_->agents.push_back(AgentState{static_cast<std::uint64_t>(i + 1), {}, 0, 0});
  for (int g = 0; g < impl_->config.generations; ++g) {
    for (auto& c : impl_->frontier) c.metrics = evaluate(c, impl_->rng, impl_->config.training_rounds, impl_->config.evaluation_rounds);
    rank_candidates(impl_->frontier);
    std::sort(impl_->frontier.begin(), impl_->frontier.end(), [](const Candidate& a, const Candidate& b) { return a.selection_score > b.selection_score; });
    const int elites = std::max(2, impl_->config.population / 3);
    for (const auto& candidate : impl_->frontier) {
      bool is_dominated = false;
      for (const auto& rival : impl_->frontier) {
        if (rival.id != candidate.id && dominates(rival.metrics, candidate.metrics)) { is_dominated = true; break; }
      }
      if (!is_dominated) impl_->history.push_back(summary(candidate));
    }
    std::vector<Candidate> next; for (int i = 0; i < elites; ++i) next.push_back(impl_->frontier.at(i));
    std::uniform_int_distribution<int> pick(0, elites - 1); while (static_cast<int>(next.size()) < impl_->config.population) next.push_back(mutate(impl_->frontier.at(pick(impl_->rng)), impl_->next_id++, impl_->rng));
    impl_->frontier = std::move(next); ++impl_->generation;
  }
  for (auto& c : impl_->frontier) c.metrics = evaluate(c, impl_->rng, impl_->config.training_rounds, impl_->config.evaluation_rounds);
  rank_candidates(impl_->frontier);
  std::sort(impl_->frontier.begin(), impl_->frontier.end(), [](const Candidate& a, const Candidate& b) { return a.selection_score > b.selection_score; });
  train_population(impl_->frontier.front(), impl_->agents, impl_->rng, impl_->config.training_rounds * 3);
  impl_->history.push_back(summary(impl_->frontier.front()));
  impl_->ensure_concept_lexicon();
}

void ResearchEngine::save(const std::filesystem::path& path) const {
  // Never stream directly into a working checkpoint.  A terminated process can
  // only leave an unreferenced temporary file; the last complete state remains
  // the file at `path` until the final atomic replacement succeeds.
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  std::filesystem::path temporary = path;
  temporary += ".writing-" + std::to_string(stamp);
  std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("cannot write temporary state: " + temporary.string());
  ensure_neu_lunari_lexemes(impl_->grammar_lexemes);
  out << "LENG_STATE 17\n" << impl_->config.seed << ' ' << impl_->config.population << ' ' << impl_->config.generations << ' ' << impl_->config.training_rounds << ' ' << impl_->config.evaluation_rounds << ' ' << impl_->config.seed_concept_lexicon << '\n';
  out << impl_->generation << ' ' << impl_->next_id << ' ' << impl_->frontier.size() << '\n';
  std::ostringstream rng_state; rng_state << impl_->rng;
  out << "R " << std::quoted(rng_state.str()) << '\n';
  for (const auto& c : impl_->frontier) { out << "C " << c.id << ' ' << c.parent << ' ' << std::setprecision(17) << c.metrics.communication_success << ' ' << c.metrics.semantic_accuracy << ' ' << c.metrics.cost << ' ' << c.metrics.pronounceability << ' ' << c.metrics.grammar_complexity << ' ' << c.metrics.robustness << ' ' << c.metrics.expressiveness << ' ' << c.lexicon.size() << '\n'; for (const auto& [atom, word] : c.lexicon) out << "L " << std::quoted(atom) << ' ' << std::quoted(word) << '\n'; const auto& order = c.order.empty() ? default_order() : c.order; out << "O " << order.size(); for (const auto& atom : order) out << ' ' << std::quoted(atom); out << '\n'; out << "M " << c.morphology.size(); for (const auto& [meaning, form] : c.morphology) out << ' ' << std::quoted(meaning) << ' ' << std::quoted(form); out << '\n'; }
  out << "A " << impl_->agents.size() << '\n';
  for (const auto& agent : impl_->agents) { out << "P " << agent.id << ' ' << agent.successful_interpretations << ' ' << agent.failed_interpretations << ' ' << agent.private_reverse_lexicon.size() << '\n'; for (const auto& [word, atom] : agent.private_reverse_lexicon) out << "K " << std::quoted(word) << ' ' << std::quoted(atom) << '\n'; }
  out << "U " << impl_->human_attempts << ' ' << impl_->human_correct << ' ' << impl_->human_total_millis << ' ' << impl_->human_production_attempts << ' ' << impl_->human_production_correct << ' ' << impl_->human_interpretation_attempts << ' ' << impl_->human_interpretation_correct << '\n';
  out << "H " << impl_->history.size() << '\n'; for (const auto& h : impl_->history) out << h.id << ' ' << h.parent_id << ' ' << h.metrics.communication_success << ' ' << h.metrics.semantic_accuracy << ' ' << h.metrics.cost << ' ' << h.metrics.pronounceability << ' ' << h.metrics.grammar_complexity << ' ' << h.metrics.robustness << ' ' << h.metrics.expressiveness << ' ' << std::quoted(h.sample) << '\n';
  out << "X " << impl_->concepts.size() << '\n';
  for (const auto& entry : impl_->concepts) out << "Q " << std::quoted(entry.id) << ' ' << std::quoted(entry.external_id) << ' ' << std::quoted(entry.label) << ' ' << std::quoted(entry.part_of_speech) << ' ' << std::quoted(entry.form) << ' ' << std::quoted(entry.derivation) << ' ' << std::quoted(entry.source) << ' ' << std::quoted(entry.provenance) << ' ' << std::quoted(entry.source_revision) << ' ' << std::quoted(entry.evidence_class) << ' ' << std::quoted(entry.language_status) << ' ' << std::quoted(entry.semantics) << ' ' << std::quoted(entry.relations) << ' ' << entry.introduced_generation << ' ' << entry.validation_attempts << ' ' << entry.validation_successes << ' ' << entry.validation_failures << '\n';
  const auto& checkpoint = impl_->import_checkpoint;
  out << "I " << std::quoted(checkpoint.source_path) << ' ' << std::quoted(checkpoint.fingerprint) << ' ' << checkpoint.next_line << ' ' << checkpoint.rows_seen << ' ' << checkpoint.rows_accepted << ' ' << checkpoint.rows_rejected << ' ' << checkpoint.complete << ' ' << std::quoted(checkpoint.status) << '\n';
  const auto& word_checkpoint = impl_->german_word_list_checkpoint;
  out << "W " << std::quoted(word_checkpoint.source_path) << ' ' << std::quoted(word_checkpoint.fingerprint) << ' ' << word_checkpoint.next_line << ' ' << word_checkpoint.rows_seen << ' ' << word_checkpoint.rows_accepted << ' ' << word_checkpoint.rows_rejected << ' ' << word_checkpoint.complete << ' ' << std::quoted(word_checkpoint.status) << '\n';
  out << "G " << impl_->grammar_lexemes.size() << '\n';
  for (const auto& entry : impl_->grammar_lexemes) out << "Z " << std::quoted(entry.concept_id) << ' ' << std::quoted(entry.historic_form) << ' ' << std::quoted(entry.form) << ' ' << std::quoted(entry.language_version) << ' ' << std::quoted(entry.status) << ' ' << std::quoted(entry.source) << ' ' << std::quoted(entry.note) << ' ' << std::quoted(entry.part_of_speech) << ' ' << std::quoted(entry.meaning) << ' ' << std::quoted(entry.valency) << ' ' << std::quoted(entry.scope) << '\n';
  out.flush();
  if (!out) { std::filesystem::remove(temporary); throw std::runtime_error("failed while writing temporary state: " + temporary.string()); }
  out.close();
  // Catch a failed/truncated write before touching the last known-good state.
  std::ifstream verify(temporary, std::ios::binary);
  std::string magic; int version{};
  const bool verified = (verify >> magic >> version) && magic == "LENG_STATE" && version >= 1 && version <= 17;
  verify.close();
  if (!verified) {
    std::filesystem::remove(temporary);
    throw std::runtime_error("temporary state verification failed: " + temporary.string());
  }
#ifdef _WIN32
  if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    std::filesystem::remove(temporary);
    throw std::runtime_error("cannot atomically replace state: " + path.string());
  }
#else
  std::error_code error;
  std::filesystem::rename(temporary, path, error);
  if (error) { std::filesystem::remove(temporary); throw std::runtime_error("cannot atomically replace state: " + path.string()); }
#endif
}
void ResearchEngine::export_results(const std::filesystem::path& directory) const {
  std::filesystem::create_directories(directory);
  const auto write_text = [&directory](const char* name, const std::string& content) { std::ofstream out(directory / name); if (!out) throw std::runtime_error(std::string("cannot export ") + name); out << content; };
  write_text("language.txt", export_language());
  write_text("report.txt", report());
  write_text("communication.txt", communication_trace());
  write_text("population.txt", population_trace());
  write_text("evolutionary_lexicon.txt", lexicon_trace());
  std::ofstream archive(directory / "pareto_archive.csv");
  if (!archive) throw std::runtime_error("cannot export pareto archive");
  archive << "candidate_id,parent_id,communication_success,semantic_accuracy,bytes,pronounceability,grammar_complexity,robustness,expressiveness,sample\n";
  for (const auto& item : impl_->history) archive << item.id << ',' << item.parent_id << ',' << item.metrics.communication_success << ',' << item.metrics.semantic_accuracy << ',' << item.metrics.cost << ',' << item.metrics.pronounceability << ',' << item.metrics.grammar_complexity << ',' << item.metrics.robustness << ',' << item.metrics.expressiveness << ',' << '"' << item.sample << "\"\n";
  std::ofstream manifest(directory / "manifest.txt");
  if (!manifest) throw std::runtime_error("cannot export manifest");
  manifest << "LINGUISTISCHE EMERGENZ NG export\nstate_format=LENG_STATE 17\ngeneration=" << impl_->generation << "\ncandidates=" << impl_->frontier.size() << "\nagents=" << impl_->agents.size() << "\nconcepts=" << impl_->concepts.size() << "\n";
}

void ResearchEngine::load(const std::filesystem::path& path) {
  std::ifstream in(path); std::string magic; int version{}; if (!(in >> magic >> version) || magic != "LENG_STATE" || version < 1 || version > 17) throw std::runtime_error("unsupported or corrupt state");
  RunConfig stored{};
  if (!(in >> stored.seed >> stored.population >> stored.generations >> stored.training_rounds >> stored.evaluation_rounds)) throw std::runtime_error("corrupt configuration");
  if (version >= 15 && !(in >> stored.seed_concept_lexicon)) throw std::runtime_error("corrupt configuration");
  // The caller controls the number of *additional* generations on resume; all
  // other reproducibility parameters come from the saved experiment.
  const int requested_generations = impl_->config.generations;
  impl_->config = stored;
  impl_->config.generations = requested_generations; impl_->grammar_lexemes.clear();
  std::size_t candidates{}; if (!(in >> impl_->generation >> impl_->next_id >> candidates)) throw std::runtime_error("corrupt header"); impl_->frontier.clear(); impl_->history.clear(); impl_->agents.clear(); impl_->concepts.clear(); impl_->rebuild_concept_indexes(); impl_->import_checkpoint = {}; impl_->german_word_list_checkpoint = {}; impl_->human_attempts = 0; impl_->human_correct = 0; impl_->human_total_millis = 0; impl_->human_production_attempts = 0; impl_->human_production_correct = 0; impl_->human_interpretation_attempts = 0; impl_->human_interpretation_correct = 0; impl_->rng.seed(impl_->config.seed);
  if (version == 4) { std::string rng_line; std::getline(in >> std::ws, rng_line); if (!rng_line.starts_with("R ")) throw std::runtime_error("corrupt RNG state"); std::istringstream rng_input(rng_line.substr(2)); if (!(rng_input >> impl_->rng)) throw std::runtime_error("corrupt RNG state"); }
  if (version >= 5) { char rng_marker{}; std::string rng_text; if (!(in >> rng_marker >> std::quoted(rng_text)) || rng_marker != 'R') throw std::runtime_error("corrupt RNG state"); std::istringstream rng_input(rng_text); if (!(rng_input >> impl_->rng)) throw std::runtime_error("corrupt RNG state"); }
  for (std::size_t n = 0; n < candidates; ++n) { Candidate c; char kind{}; std::size_t count{}; if (!(in >> kind >> c.id >> c.parent >> c.metrics.communication_success >> c.metrics.semantic_accuracy >> c.metrics.cost >> c.metrics.pronounceability >> c.metrics.grammar_complexity >> c.metrics.robustness >> c.metrics.expressiveness >> count) || kind != 'C') throw std::runtime_error("corrupt candidate"); for (std::size_t i = 0; i < count; ++i) { std::string atom, word; if (!(in >> kind >> std::quoted(atom) >> std::quoted(word)) || kind != 'L') throw std::runtime_error("corrupt lexicon"); c.lexicon.emplace(std::move(atom), std::move(word)); } if (version >= 7) { if (!(in >> kind >> count) || kind != 'O') throw std::runtime_error("corrupt grammar order"); std::set<std::string> fields; for (std::size_t i = 0; i < count; ++i) { std::string field; if (!(in >> std::quoted(field)) || !fields.insert(field).second) throw std::runtime_error("corrupt grammar field"); if (version >= 9) c.order.push_back(std::move(field)); } if (version >= 9) { if (count != default_order().size()) throw std::runtime_error("corrupt grammar order"); for (const auto& field : default_order()) if (!fields.contains(field)) throw std::runtime_error("incomplete grammar order"); } else c.order = default_order(); } else c.order = default_order(); if (version >= 14) { if (!(in >> kind >> count) || kind != 'M' || count != 5) throw std::runtime_error("corrupt morphology inventory"); for (std::size_t i = 0; i < count; ++i) { std::string meaning, form; if (!(in >> std::quoted(meaning) >> std::quoted(form))) throw std::runtime_error("corrupt morphology form"); c.morphology.emplace(std::move(meaning), std::move(form)); } } if (version < 9) { const Candidate required = make_candidate(0, 0, impl_->rng); std::set<std::string> used; for (const auto& [_, word] : c.lexicon) used.insert(word); for (const auto& [atom, _] : required.lexicon) if (!c.lexicon.contains(atom)) c.lexicon.emplace(atom, new_word(impl_->rng, used)); c.order = default_order(); } ensure_morphology(c, impl_->rng); if (version == 1) c.metrics.grammar_complexity = 14.0; impl_->frontier.push_back(std::move(c)); }
  char marker{};
  if (version >= 3) { std::size_t agent_count{}; if (!(in >> marker >> agent_count) || marker != 'A') throw std::runtime_error("corrupt agent population"); for (std::size_t i = 0; i < agent_count; ++i) { AgentState agent; std::size_t knowledge_count{}; if (!(in >> marker >> agent.id >> agent.successful_interpretations >> agent.failed_interpretations >> knowledge_count) || marker != 'P') throw std::runtime_error("corrupt agent record"); for (std::size_t k = 0; k < knowledge_count; ++k) { std::string word, atom; if (!(in >> marker >> std::quoted(word) >> std::quoted(atom)) || marker != 'K') throw std::runtime_error("corrupt agent knowledge"); agent.private_reverse_lexicon.emplace(std::move(word), std::move(atom)); } impl_->agents.push_back(std::move(agent)); } }
  if (version >= 6) { if (!(in >> marker >> impl_->human_attempts >> impl_->human_correct >> impl_->human_total_millis) || marker != 'U') throw std::runtime_error("corrupt human study"); if (version >= 8 && !(in >> impl_->human_production_attempts >> impl_->human_production_correct >> impl_->human_interpretation_attempts >> impl_->human_interpretation_correct)) throw std::runtime_error("corrupt human study metrics"); }
  std::size_t historic{}; if (!(in >> marker >> historic) || marker != 'H') throw std::runtime_error(std::string("corrupt history marker: ") + marker); for (std::size_t i = 0; i < historic; ++i) { CandidateSummary h; if (!(in >> h.id >> h.parent_id >> h.metrics.communication_success >> h.metrics.semantic_accuracy >> h.metrics.cost >> h.metrics.pronounceability >> h.metrics.grammar_complexity >> h.metrics.robustness >> h.metrics.expressiveness >> std::quoted(h.sample))) throw std::runtime_error("corrupt historical record"); impl_->history.push_back(std::move(h)); }
  if (version >= 10) {
    std::size_t concept_count{};
    if (!(in >> marker >> concept_count) || marker != 'X') throw std::runtime_error("corrupt concept lexicon");
    impl_->concepts.reserve(concept_count);
    for (std::size_t i = 0; i < concept_count; ++i) {
      ConceptEntry entry;
      if (version >= 12) {
        if (!(in >> marker >> std::quoted(entry.id) >> std::quoted(entry.external_id) >> std::quoted(entry.label) >> std::quoted(entry.part_of_speech) >> std::quoted(entry.form) >> std::quoted(entry.derivation) >> std::quoted(entry.source) >> std::quoted(entry.provenance) >> std::quoted(entry.source_revision) >> std::quoted(entry.evidence_class) >> std::quoted(entry.language_status) >> std::quoted(entry.semantics) >> std::quoted(entry.relations) >> entry.introduced_generation >> entry.validation_attempts >> entry.validation_successes >> entry.validation_failures) || marker != 'Q') throw std::runtime_error("corrupt V12 concept entry");
      } else {
        if (!(in >> marker >> std::quoted(entry.id) >> std::quoted(entry.label) >> std::quoted(entry.part_of_speech) >> std::quoted(entry.form) >> std::quoted(entry.derivation) >> std::quoted(entry.source))) throw std::runtime_error("corrupt concept entry");
        if (version >= 11) { if (!(in >> std::quoted(entry.provenance))) throw std::runtime_error("corrupt concept provenance"); } else entry.provenance = "migrated-from-LENG_STATE-10";
        if (!(in >> std::quoted(entry.semantics) >> std::quoted(entry.relations) >> entry.introduced_generation >> entry.validation_attempts >> entry.validation_successes) || marker != 'Q') throw std::runtime_error("corrupt concept entry");
        entry.source_revision = "migrated-v" + std::to_string(version);
        entry.evidence_class = entry.derivation == "relation-compound" || entry.derivation == "compound" ? "simulated-relation" : "agent-hypothesis";
        entry.language_status = entry.validation_successes == entry.validation_attempts && entry.validation_attempts > 0 ? "communicatively-validated" : "realised";
        entry.validation_failures = entry.validation_attempts - entry.validation_successes;
      }
      impl_->concepts.push_back(std::move(entry));
    }
  }
  if (version >= 12) {
    auto& checkpoint = impl_->import_checkpoint;
    if (!(in >> marker >> std::quoted(checkpoint.source_path) >> std::quoted(checkpoint.fingerprint) >> checkpoint.next_line >> checkpoint.rows_seen >> checkpoint.rows_accepted >> checkpoint.rows_rejected >> checkpoint.complete >> std::quoted(checkpoint.status)) || marker != 'I') throw std::runtime_error("corrupt V12 import checkpoint");
  }
  if (version >= 13) {
    auto& checkpoint = impl_->german_word_list_checkpoint;
    if (!(in >> marker >> std::quoted(checkpoint.source_path) >> std::quoted(checkpoint.fingerprint) >> checkpoint.next_line >> checkpoint.rows_seen >> checkpoint.rows_accepted >> checkpoint.rows_rejected >> checkpoint.complete >> std::quoted(checkpoint.status)) || marker != 'W') throw std::runtime_error("corrupt V13 German word-list checkpoint");
  }
  if (version >= 16) { std::size_t grammar_count{}; if (!(in >> marker >> grammar_count) || marker != 'G') throw std::runtime_error("corrupt grammar lexicon"); for (std::size_t i = 0; i < grammar_count; ++i) { GrammarLexeme entry; if (!(in >> marker >> std::quoted(entry.concept_id) >> std::quoted(entry.historic_form) >> std::quoted(entry.form) >> std::quoted(entry.language_version) >> std::quoted(entry.status) >> std::quoted(entry.source) >> std::quoted(entry.note)) || marker != 'Z') throw std::runtime_error("corrupt grammar lexeme"); if (version >= 17 && !(in >> std::quoted(entry.part_of_speech) >> std::quoted(entry.meaning) >> std::quoted(entry.valency) >> std::quoted(entry.scope))) throw std::runtime_error("corrupt grammar lexeme metadata"); impl_->grammar_lexemes.push_back(std::move(entry)); } ensure_neu_lunari_lexemes(impl_->grammar_lexemes); } else ensure_neu_lunari_lexemes(impl_->grammar_lexemes);
  impl_->rebuild_concept_indexes();
  if (impl_->agents.empty() && !impl_->frontier.empty()) { for (int i = 0; i < impl_->config.population; ++i) impl_->agents.push_back(AgentState{static_cast<std::uint64_t>(i + 1), {}, 0, 0}); train_population(impl_->frontier.front(), impl_->agents, impl_->rng, impl_->config.training_rounds * 3); }
  if (!impl_->frontier.empty()) impl_->ensure_concept_lexicon();
}

std::size_t ResearchEngine::import_concepts(const std::filesystem::path& tsv_path) {
  std::size_t total{};
  for (;;) { const auto progress = import_concepts_batch(tsv_path, 5000); total += progress.rows_accepted; if (progress.complete) return total; }
}

ImportProgress ResearchEngine::import_concepts_batch(const std::filesystem::path& tsv_path, std::size_t max_rows) {
  if (impl_->frontier.empty()) throw std::runtime_error("run an experiment before importing concepts");
  if (max_rows == 0) throw std::invalid_argument("import batch size must be positive");
  impl_->ensure_concept_lexicon();
  const auto canonical_path = std::filesystem::absolute(tsv_path).lexically_normal().string();
  const auto fingerprint = stable_fingerprint(tsv_path);
  auto& checkpoint = impl_->import_checkpoint;
  if (checkpoint.source_path != canonical_path || checkpoint.fingerprint != fingerprint) checkpoint = {canonical_path, fingerprint, 1, 0, 0, 0, false, "running"};
  ImportProgress result{}; result.source_fingerprint = fingerprint;
  if (checkpoint.complete) { result.complete = true; result.source_line = checkpoint.next_line; result.status = "source already complete"; return result; }
  std::ifstream input(tsv_path); if (!input) throw std::runtime_error("cannot read concept import: " + tsv_path.string());
  std::string line; std::size_t line_number{}; std::size_t processed{};
  std::vector<std::string> touched_labels;
  const auto append_once = [](std::string& value, const std::string& addition, const char* separator) { if (!addition.empty() && value.find(addition) == std::string::npos) value += (value.empty() ? "" : separator) + addition; };
  while (std::getline(input, line)) {
    ++line_number;
    if (line_number < checkpoint.next_line) continue;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (trim(line).empty() || trim(line).starts_with('#')) { checkpoint.next_line = line_number + 1; continue; }
    const auto fields = split_tsv(line);
    if (line_number == 1 && !fields.empty() && (fields[0] == "label" || fields[0] == "external_id")) { checkpoint.next_line = 2; continue; }
    if (fields.size() != 6 && fields.size() != 9) throw std::runtime_error("concept import line " + std::to_string(line_number) + " must contain 6 legacy or 9 canonical TSV columns");
    ++processed; ++checkpoint.rows_seen; ++result.rows_seen;
    const bool canonical = fields.size() == 9;
    const std::string external_id = canonical ? fields[0] : "";
    const std::string label = canonical ? fields[1] : fields[0];
    const std::string part = canonical ? fields[2] : fields[1];
    const std::string semantics = canonical ? fields[3] : fields[2];
    const std::string relations = canonical ? fields[4] : fields[3];
    const std::string source = canonical ? fields[5] : fields[4];
    const std::string provenance = canonical ? fields[6] : fields[5];
    const std::string revision = canonical ? fields[7] : "legacy-tsv";
    const std::string evidence = canonical ? fields[8] : "external-attested";
    if (label.empty() || part.empty() || semantics.empty() || source.empty() || provenance.empty() || (canonical && external_id.empty())) { ++checkpoint.rows_rejected; ++result.concepts_rejected; checkpoint.next_line = line_number + 1; continue; }
    if (canonical && source == "Wikidata" && (external_id.size() < 2 || external_id.front() != 'Q')) { ++checkpoint.rows_rejected; ++result.concepts_rejected; checkpoint.next_line = line_number + 1; continue; }
    const std::string identity = external_id.empty() ? canonical_concept_key(label) : external_id;
    ConceptEntry* existing = impl_->find_concept(identity);
    if (existing == nullptr && external_id.empty()) existing = impl_->find_concept(label);
    if (existing) { append_once(existing->semantics, semantics, ";"); append_once(existing->relations, relations, ";"); append_once(existing->source, source, "+"); append_once(existing->provenance, provenance, " | "); append_once(existing->source_revision, revision, "+"); ++result.concepts_enriched; }
    else {
      ConceptEntry entry{}; entry.external_id = external_id; entry.label = label; entry.part_of_speech = part; entry.form = new_concept_word(impl_->rng, impl_->used_forms); entry.derivation = "imported-root"; entry.source = source; entry.provenance = provenance; entry.source_revision = revision; entry.evidence_class = evidence; entry.language_status = "realised"; entry.semantics = semantics; entry.relations = relations; entry.introduced_generation = impl_->generation; impl_->add_concept(std::move(entry)); ++result.concepts_created;
    }
    touched_labels.push_back(label);
    ++checkpoint.rows_accepted; ++result.rows_accepted; checkpoint.next_line = line_number + 1;
    if (processed >= max_rows) break;
  }
  // A full batch can end exactly on the final data line without a failed
  // getline() call.  Peek once so completion does not require a pointless
  // extra autonomous step.
  if (input.eof() || input.peek() == std::char_traits<char>::eof()) { checkpoint.complete = true; checkpoint.status = "complete"; }
  // Build a regular relational family only when both independently-attested
  // endpoints are known.  The entry remains marked simulated-relation; the
  // external edge itself stays in each source entry's relation evidence.
  for (const auto& source_label : touched_labels) {
    const auto* source = impl_->find_concept(source_label);
    if (!source) continue;
    const std::string source_id = source->id, source_name = source->label, source_form = source->form, source_semantics = source->semantics, source_relations = source->relations;
    std::istringstream relation_stream(source_relations);
    for (std::string relation; std::getline(relation_stream, relation, ';');) {
      const auto colon = relation.find(':'); if (colon == std::string::npos) continue;
      std::istringstream targets(relation.substr(colon + 1));
      for (std::string target_label; std::getline(targets, target_label, ',');) {
        target_label = trim(std::move(target_label)); const auto* target = impl_->find_concept(target_label);
        if (!target || target->id == source_id) continue;
        const std::string compound_label = source_name + "-" + target->label;
        if (impl_->find_concept(compound_label)) continue;
        const std::string target_id = target->id, target_form = target->form;
        ConceptEntry compound{}; compound.label = compound_label; compound.part_of_speech = "noun"; compound.form = source_form + "-" + target_form; compound.derivation = "relation-compound"; compound.source = "simulated-experience"; compound.provenance = "derived-from:" + source_id + ";relation:" + relation; compound.source_revision = "rule-v2"; compound.evidence_class = "simulated-relation"; compound.language_status = "realised"; compound.semantics = "composed;" + source_semantics; compound.relations = "derived-from:" + source_id + ";linked-to:" + target_id; compound.introduced_generation = impl_->generation; impl_->add_concept(std::move(compound)); ++result.relations_created;
      }
    }
  }
  result.source_line = checkpoint.next_line; result.complete = checkpoint.complete; result.status = checkpoint.status;
  // Validation is deliberately separate from external intake but advances in
  // the same bounded operation.  This never labels a merely imported row as
  // communicatively valid without an independent receiver attempt.
  const auto validated_now = validate_concepts(result.concepts_created + result.concepts_enriched);
  result.status += "; validation-trials=" + std::to_string(validated_now);
  return result;
}

ImportProgress ResearchEngine::import_german_word_list_batch(const std::filesystem::path& word_list_path,
                                                              std::size_t max_words) {
  if (impl_->frontier.empty()) throw std::runtime_error("run an experiment before importing word forms");
  if (max_words == 0) throw std::invalid_argument("word-list batch size must be positive");
  impl_->ensure_concept_lexicon();
  const auto canonical_path = std::filesystem::absolute(word_list_path).lexically_normal().string();
  const auto fingerprint = stable_fingerprint(word_list_path);
  auto& checkpoint = impl_->german_word_list_checkpoint;
  if (checkpoint.source_path != canonical_path || checkpoint.fingerprint != fingerprint)
    checkpoint = {canonical_path, fingerprint, 1, 0, 0, 0, false, "running"};
  ImportProgress result{}; result.source_fingerprint = fingerprint;
  if (checkpoint.complete) { result.complete = true; result.source_line = checkpoint.next_line; result.status = "local German word list already complete"; return result; }
  std::ifstream input(word_list_path); if (!input) throw std::runtime_error("cannot read German word list: " + word_list_path.string());
  std::string line; std::size_t line_number{}, processed{};
  while (std::getline(input, line)) {
    ++line_number;
    if (line_number < checkpoint.next_line) continue;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::string label = trim(std::move(line));
    if (label.empty() || label.starts_with('#')) { checkpoint.next_line = line_number + 1; continue; }
    ++processed; ++checkpoint.rows_seen; ++result.rows_seen;
    const std::string local_id = "LOCAL-DE:" + canonical_concept_key(label);
    // The source is a word-form list, not a semantic authority.  It therefore
    // receives a stable local ID and a separate evidence class; it must never
    // be counted as an externally attested or semantically validated concept.
    if (!impl_->find_concept(local_id)) {
      ConceptEntry entry{};
      // `external_id` stores a namespaced source-local identity here. It is
      // not a claim of external attestation; evidence_class below remains the
      // authoritative provenance category and prevents qualification.
      entry.external_id = local_id;
      entry.label = label;
      entry.part_of_speech = "unknown";
      entry.form = new_concept_word(impl_->rng, impl_->used_forms);
      entry.derivation = "local-word-form";
      entry.source = "user-provided-german-word-list";
      entry.provenance = "local-file:" + word_list_path.filename().string() + ";licence=unknown;meaning=unverified";
      entry.source_revision = fingerprint;
      entry.evidence_class = "agent-hypothesis";
      entry.language_status = "realised";
      entry.semantics = "lexical-form;language=de;meaning=unverified";
      entry.relations = "";
      entry.introduced_generation = impl_->generation;
      impl_->add_concept(std::move(entry));
      ++result.concepts_created;
    } else ++result.concepts_enriched;
    ++checkpoint.rows_accepted; ++result.rows_accepted; checkpoint.next_line = line_number + 1;
    if (processed >= max_words) break;
  }
  if (input.eof() || input.peek() == std::char_traits<char>::eof()) { checkpoint.complete = true; checkpoint.status = "complete"; }
  result.source_line = checkpoint.next_line; result.complete = checkpoint.complete; result.status = checkpoint.status;
  return result;
}

std::size_t ResearchEngine::validate_concepts(std::size_t max_concepts) {
  if (impl_->concepts.empty()) return 0;
  // Sender and receiver have deliberately separate maps.  Both learn atomic
  // forms from the persistent lexicon, while the receiver never sees the
  // composed candidate before the trial.  The semantic reference is rebuilt
  // from the component labels, not the candidate ID or its surface label.
  std::map<std::string, std::string> sender_atoms, receiver_atoms;
  if (impl_->agents.size() < 2) throw std::runtime_error("concept validation requires two persistent agents");
  AgentState& sender_agent = impl_->agents.at(0);
  AgentState& receiver_agent = impl_->agents.at(1);
  for (const auto& entry : impl_->concepts) {
    if (entry.derivation == "root" || entry.derivation == "imported-root") {
      sender_atoms.emplace(entry.form, entry.label);
      receiver_atoms.emplace(entry.form, entry.label);
    }
  }
  std::size_t validated{};
  for (auto& entry : impl_->concepts) {
    if (max_concepts != 0 && validated >= max_concepts) break;
    if (entry.derivation != "compound" && entry.derivation != "relation-compound") continue;
    if (entry.language_status == "communicatively-validated") continue;
    const auto expected = entry.label;
    const bool sender_can_encode = !entry.form.empty() && entry.form.find('-') != std::string::npos;
    bool receiver_can_decode = sender_can_encode;
    std::vector<std::string> recovered;
    std::istringstream parts(entry.form);
    for (std::string part; receiver_can_decode && std::getline(parts, part, '-');) {
      const auto learned = receiver_atoms.find(part);
      if (learned == receiver_atoms.end()) receiver_can_decode = false;
      else recovered.push_back(learned->second);
    }
    const bool independent_match = receiver_can_decode && join(recovered, "-") == expected;
    constexpr std::uint64_t kTrials = 6;
    entry.validation_attempts += kTrials;
    if (sender_can_encode && independent_match) { entry.validation_successes += kTrials; sender_agent.successful_interpretations += kTrials; receiver_agent.successful_interpretations += kTrials; }
    else { entry.validation_failures += kTrials; sender_agent.failed_interpretations += kTrials; receiver_agent.failed_interpretations += kTrials; }
    entry.language_status = sender_can_encode && independent_match ? "communicatively-validated" : "rejected";
    ++validated;
  }
  return validated;
}

std::string ResearchEngine::report() const { std::ostringstream out; const auto compute = inspect_compute(); out << "Research generation: " << impl_->generation << "\nCandidates: " << impl_->frontier.size() << "\nOpen concept lexicon: " << impl_->concepts.size() << " entries\n" << lexicon_status() << "\nCompute: " << compute.diagnostic; for (const auto& device : compute.devices) out << "\nOpenCL device: " << device; if (!impl_->frontier.empty()) { const auto& m = impl_->frontier.front().metrics; out << "\nBest candidate: " << impl_->frontier.front().id << " (parent " << impl_->frontier.front().parent << ")\nCommunication success (unseen compositions): " << m.communication_success << "\nSemantic accuracy: " << m.semantic_accuracy << "\nRobustness under one valid actor-token substitution: " << m.robustness << "\nMean bytes/message: " << m.cost << "\nPronounceability estimate: " << m.pronounceability << "\nGrammar complexity (1=canonical regular order): " << m.grammar_complexity << "\nPareto archive records: " << impl_->history.size(); } return out.str(); }
const std::vector<CandidateSummary>& ResearchEngine::archive() const { return impl_->history; }
std::string ResearchEngine::export_language() const { if (impl_->frontier.empty()) return "No language generated."; std::ostringstream out; out << "Language candidate " << impl_->frontier.front().id << "\nGrammar order: [" << grammar_description(impl_->frontier.front()) << "]\n"; for (const auto& [atom, word] : impl_->frontier.front().lexicon) out << atom << " = " << word << '\n'; return out.str(); }
std::string ResearchEngine::lexicon_trace() const {
  std::ostringstream out;
  out << "Autonomous lexicon — persistent concept graph\n";
  out << lexicon_status() << "\n";
  out << "Morphology rule version: " << kConceptMorphologyRuleVersion << "\n";
  out << "Validation: receiver agents reconstruct atomic forms and evaluate unseen composed forms; opaque concept IDs are not transmitted.\n\n";
  for (const auto& entry : impl_->concepts) {
    out << entry.id << "  " << entry.form << "\n"
        << "  meaning: " << entry.label << "  part of speech: " << entry.part_of_speech << "\n"
        << "  semantics: " << entry.semantics << "\n"
        << "  relations: " << entry.relations << "\n"
        << "  formation: " << entry.derivation << "  evidence: " << entry.evidence_class << "  source: " << entry.source << "  revision: " << entry.source_revision << "\n"
        << "  provenance: " << entry.provenance << "  external identity: " << (entry.external_id.empty() ? "<local>" : entry.external_id) << "  generation: " << entry.introduced_generation << "\n"
        << "  language status: " << entry.language_status << "  agent validation: " << entry.validation_successes << "/" << entry.validation_attempts << " successes; failures=" << entry.validation_failures << "\n\n";
  }
  return out.str();
}
std::string ResearchEngine::lexicon_status() const {
  std::size_t external{}, qualified_external{}, simulated{}, hypotheses{}, local_german_forms{}, realised{}, validated{}, pending{}, rejected{};
  for (const auto& entry : impl_->concepts) {
    if (entry.evidence_class == "external-attested") { ++external; if (externally_qualified(entry)) ++qualified_external; }
    else if (entry.evidence_class == "simulated-relation") ++simulated;
    else ++hypotheses;
    if (entry.source == "user-provided-german-word-list") ++local_german_forms;
    if (entry.language_status == "communicatively-validated") ++validated;
    else if (entry.language_status == "realised") ++realised;
    else if (entry.language_status == "rejected") ++rejected;
    else ++pending;
  }
  const auto& checkpoint = impl_->import_checkpoint;
  const auto& local_checkpoint = impl_->german_word_list_checkpoint;
  std::ostringstream out;
  out << "Lexicon status\nunique concepts=" << impl_->concepts.size() << "\nimported concepts=" << external << "\nqualified external concepts=" << qualified_external << "\nautomatically developed wordforms=" << (realised + validated) << "\nagentically validated concepts=" << validated << "\nexternal-attested=" << external << "\nsimulated-relations=" << simulated << "\nagent-hypotheses=" << hypotheses << "\nlocal German word forms (meaning unverified)=" << local_german_forms << "\nlanguage-realised=" << realised << "\ncommunicatively-validated=" << validated << "\npending=" << pending << "\nrejected=" << rejected << "\nimport-status=" << checkpoint.status << "\nimport-source=" << (checkpoint.source_path.empty() ? "<none>" : checkpoint.source_path) << "\nimport-next-line=" << checkpoint.next_line << "\nimport-accepted=" << checkpoint.rows_accepted << "\nimport-rejected=" << checkpoint.rows_rejected << "\nlocal-word-list-status=" << local_checkpoint.status << "\nlocal-word-list-source=" << (local_checkpoint.source_path.empty() ? "<none>" : local_checkpoint.source_path) << "\nlocal-word-list-next-line=" << local_checkpoint.next_line << "\nlocal-word-list-accepted=" << local_checkpoint.rows_accepted << "\nlocal-word-list-rejected=" << local_checkpoint.rows_rejected;
  return out.str();
}
std::string ResearchEngine::search_lexicon(const std::string& query, std::size_t limit) const {
  const auto needle = canonical_concept_key(query); if (needle.empty()) return "Lexicon search\nEnter a label, identity, source, or form.";
  std::ostringstream out; out << "Lexicon search: " << query << "\n"; std::size_t matches{};
  for (const auto& entry : impl_->concepts) {
    const auto matches_text = [&needle](const std::string& value) { return canonical_concept_key(value).find(needle) != std::string::npos; };
    if (!matches_text(entry.label) && !matches_text(entry.external_id) && !matches_text(entry.source) && !matches_text(entry.form)) continue;
    out << entry.id << "  " << entry.label << "  form=" << entry.form << "  evidence=" << entry.evidence_class << "  language=" << entry.language_status << "\n";
    if (++matches >= limit) break;
  }
  if (matches == 0) out << "No matching persistent concept.\n"; else out << "matches=" << matches << (matches == limit ? " (limit reached)" : "");
  return out.str();
}
std::string ResearchEngine::dictionary_page(const std::string& query, std::size_t page, std::size_t page_size) const {
  if (page_size == 0) throw std::invalid_argument("dictionary page size must be positive");
  const auto needle = canonical_concept_key(query);
  std::vector<const ConceptEntry*> matches;
  matches.reserve(impl_->concepts.size());
  for (const auto& entry : impl_->concepts) {
    const auto contains = [&needle](const std::string& value) {
      return needle.empty() || canonical_concept_key(value).find(needle) != std::string::npos;
    };
    if (contains(entry.label) || contains(entry.external_id) || contains(entry.form) || contains(entry.semantics)) matches.push_back(&entry);
  }
  const std::size_t pages = std::max<std::size_t>(1, (matches.size() + page_size - 1) / page_size);
  std::ostringstream out;
  out << "Dictionary / Wörterbuch\nSearch / Suche: " << (query.empty() ? "all persistent concepts" : query)
      << "\nPage / Seite: " << (page + 1) << " of " << pages << "  |  Matches / Treffer: " << matches.size() << "\n\n";
  if (page >= pages || matches.empty()) {
    out << (matches.empty() ? "No matching persistent concept / Kein passendes persistentes Konzept." : "No further page / Keine weitere Seite.");
    return out.str();
  }
  const std::size_t first = page * page_size;
  const std::size_t last = std::min(matches.size(), first + page_size);
  for (std::size_t i = first; i < last; ++i) {
    const auto& entry = *matches[i];
    bool german_fallback = false;
    const std::string german = localize_label(entry.label, "de", german_fallback);
    out << (i + 1) << ". " << entry.form << "\n"
        << "   English: " << entry.label << "  |  Deutsch: " << german << (german_fallback ? " (source label)" : "") << "\n"
        << "   Part of speech / Wortart: " << entry.part_of_speech << "  |  Status: " << entry.language_status << "\n"
        << "   Evidence / Evidenz: " << entry.evidence_class
        << "  |  Source / Quelle: " << entry.source
        << (entry.external_id.empty() ? "" : "  |  ID: " + entry.external_id) << "\n\n";
  }
  out << "Use Previous page / Next page to browse. Enter a word, form, QID, or semantic term and press Dictionary / Wörterbuch to start a new search.";
  return out.str();
}
std::string ResearchEngine::lexicon_benchmark() const {
  std::size_t payload_bytes{};
  for (const auto& entry : impl_->concepts) payload_bytes += entry.id.size() + entry.external_id.size() + entry.label.size() + entry.part_of_speech.size() + entry.form.size() + entry.derivation.size() + entry.source.size() + entry.provenance.size() + entry.source_revision.size() + entry.evidence_class.size() + entry.language_status.size() + entry.semantics.size() + entry.relations.size();
  const std::size_t probes = std::min<std::size_t>(1000, impl_->concepts.size());
  const auto started = std::chrono::steady_clock::now(); std::size_t hits{};
  for (std::size_t i = 0; i < probes; ++i) {
    const auto& entry = impl_->concepts[(i * 7919U) % impl_->concepts.size()];
    if (impl_->by_identity.contains(entry.external_id.empty() ? entry.id : entry.external_id)) ++hits;
  }
  const auto lookup_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started).count();
  std::size_t communication_candidates{}; for (const auto& entry : impl_->concepts) if (entry.language_status == "communicatively-validated") ++communication_candidates;
  std::ostringstream out;
  out << "Lexicon benchmark\nconcepts=" << impl_->concepts.size() << "\nestimated persistent string payload bytes=" << payload_bytes << "\nindex entries=" << impl_->by_identity.size() << "\nindex lookup probes=" << probes << "\nindex lookup hits=" << hits << "\nindex lookup micros=" << lookup_us << "\nindex lookup micros per probe=" << (probes == 0 ? 0.0 : static_cast<double>(lookup_us) / probes) << "\ncommunication-valid candidates=" << communication_candidates << "\nNote: memory is a conservative persistent-string payload estimate; allocator and process overhead are intentionally not misreported as exact RAM.";
  return out.str();
}

std::string ResearchEngine::translate(const std::string& text, const std::string& target_language) const {
  if (target_language != "de" && target_language != "en" && target_language != "constructed")
    throw std::invalid_argument("translation target must be de, en, or constructed");
  const auto input = words(text);
  if (input.empty()) throw std::invalid_argument("enter text to translate");
  if (impl_->frontier.empty()) throw std::runtime_error("generate or load a language before translating");
  const Candidate& language = impl_->frontier.front();
  std::map<std::string, std::string> form_to_label;
  std::map<std::string, std::string> label_to_form;
  std::map<std::string, std::string> local_word_to_form;
  for (const auto& [atom, form] : language.lexicon) {
    const auto separator = atom.find(':');
    const std::string label = atom.substr(separator + 1);
    form_to_label.emplace(canonical_concept_key(form), label);
    label_to_form.emplace(canonical_concept_key(label), form);
  }
  for (const auto& entry : impl_->concepts) {
    form_to_label.emplace(canonical_concept_key(entry.form), entry.label);
    label_to_form.emplace(canonical_concept_key(entry.label), entry.form);
    if (entry.derivation == "local-word-form") local_word_to_form.emplace(canonical_concept_key(entry.label), entry.form);
  }
  std::map<std::string, std::string> german_to_label;
  for (const auto& [english, german] : german_terms()) german_to_label.emplace(canonical_concept_key(german), english);

  std::vector<std::string> labels;
  std::vector<std::string> unresolved;
  bool saw_constructed = false;
  std::vector<std::string> constructed_sequence;
  for (std::size_t token_index = 0; token_index < input.size(); ++token_index) {
    const auto& token = input[token_index];
    const std::string previous = token_index == 0 ? "" : input[token_index - 1];
    // Persistent dictionary entries always win over heuristic morphology.
    // Otherwise ordinary nouns such as “Bett” would be misread as a verb
    // merely because they end in -t.
    if (const auto found = form_to_label.find(token); found != form_to_label.end()) { labels.push_back(found->second); saw_constructed = true; if (target_language == "constructed") constructed_sequence.push_back(token); continue; }
    if (const auto found = local_word_to_form.find(token); found != local_word_to_form.end()) { labels.push_back(token); if (target_language == "constructed") constructed_sequence.push_back(found->second); continue; }
    if (target_language == "constructed") if (const auto verb = analyse_german_verb(token, previous)) { labels.push_back(verb->root + ";" + (verb->number.empty() ? "infinitive" : verb->number) + ";" + verb->time); constructed_sequence.push_back(productive_verb_form(impl_->frontier.front(), *verb)); continue; }
    if (const auto found = label_to_form.find(token); found != label_to_form.end()) { labels.push_back(found->first); if (target_language == "constructed") constructed_sequence.push_back(found->second); continue; }
    if (const auto found = german_to_label.find(token); found != german_to_label.end()) { labels.push_back(found->second); if (target_language == "constructed" && label_to_form.contains(found->second)) constructed_sequence.push_back(label_to_form.at(found->second)); continue; }
    unresolved.push_back(token);
  }
  std::ostringstream out;
  out << "Semantic translator\n";
  out << "Input representation: " << (saw_constructed ? "constructed language / mixed" : "English or German semantic labels") << "\n";
  if (!unresolved.empty()) out << "Unresolved tokens (preserved, not guessed): " << join(unresolved, ", ") << "\n";
  if (labels.empty()) { out << "No persistent concept or grammar term was recognized."; return out.str(); }
  out << "Recognized concepts: " << join(labels, " | ") << "\n";
  if (target_language == "constructed") {
    out << "Constructed language: " << join(constructed_sequence, " ");
    if (std::any_of(labels.begin(), labels.end(), [](const std::string& item) { return item.find(';') != std::string::npos; })) out << "\nProductive morphology: root + person-3 + number + time (not a lexical lookup).";
  } else {
    bool fallback = false;
    std::vector<std::string> rendered;
    for (const auto& label : labels) rendered.push_back(localize_label(label, target_language, fallback));
    out << (target_language == "de" ? "Deutsch: " : "English: ") << join(rendered, " ");
    if (fallback) out << "\nNote: labels without a reviewed German gloss remain in their source-language form.";
  }
  return out.str();
}

std::string ResearchEngine::translate_meaning(const std::string& text, const std::string& target_language) const {
  if (target_language != "de" && target_language != "en" && target_language != "constructed")
    throw std::invalid_argument("meaning translation target must be de, en, or constructed");
  if (text.empty()) throw std::invalid_argument("enter a semantic-core sentence or a Lunari meaning frame");
  if (impl_->frontier.empty()) throw std::runtime_error("generate or load a language before translating a meaning");
  const Candidate& language = impl_->frontier.front();
  if (target_language == "constructed") {
    const auto meaning = analyse_meaning_sentence(text);
    if (!meaning) {
      throw std::invalid_argument("semantic-core analysis needs a known actor, action, and object; use the reversible text channel for unanalysed text");
    }
    std::ostringstream out;
    out << "Lunari meaning channel\nLunari frame:\n" << encode(language, *meaning)
        << "\n-- end Lunari frame --\n"
        << "Source-independent semantic frame:\n" << render_meaning(*meaning, "en")
        << "\nGuarantee: the Lunari frame contains evolved semantic-field forms, not German word forms.";
    return out.str();
  }
  std::map<std::string, std::string> receiver;
  for (const auto& [atom, form] : language.lexicon) receiver.emplace(form, atom);
  Meaning recovered;
  if (!decode(receiver, text, recovered)) {
    throw std::invalid_argument("not a complete Lunari semantic frame; all 15 semantic-field forms are required");
  }
  std::ostringstream out;
  out << "Lunari meaning channel\n" << render_meaning(recovered, target_language)
      << "\nDecoded independently from Lunari semantic-field forms; no German source text was consulted.";
  return out.str();
}

std::string ResearchEngine::translate_text(const std::string& text, const std::string& target_language) {
  if (target_language != "de" && target_language != "en" && target_language != "constructed")
    throw std::invalid_argument("translation target must be de, en, or constructed");
  if (text.empty()) throw std::invalid_argument("enter text to translate");
  if (impl_->frontier.empty()) throw std::runtime_error("generate or load a language before translating");
  std::map<std::string, std::string> label_to_form, form_to_label;
  for (const auto& entry : impl_->concepts) {
    label_to_form.emplace(canonical_concept_key(entry.label), entry.form);
    form_to_label.emplace(entry.form, entry.label);
  }
  for (const auto& [atom, form] : impl_->frontier.front().lexicon) {
    const auto separator = atom.find(':');
    label_to_form.emplace(atom.substr(separator + 1), form);
    form_to_label.emplace(form, atom.substr(separator + 1));
  }
  std::ostringstream rendered;
  std::size_t dictionary_tokens{}, learned_tokens{}, decoded_tokens{}, unresolved{};
  for (const auto& [lexical, piece] : text_segments(text)) {
    if (!lexical) { rendered << piece; continue; }
    if (target_language == "constructed") {
      const std::string key = canonical_concept_key(piece);
      auto known = label_to_form.find(key);
      if (known == label_to_form.end()) {
        // Text intake is lexical only: it preserves a new word and a Lunari
        // form, but never fabricates a meaning, part of speech, or English
        // gloss. The checkpoint is saved by the CLI/UI after this operation.
        ConceptEntry entry{};
        entry.external_id = "TEXT-DE:" + key;
        entry.label = piece;
        entry.part_of_speech = "unknown";
        entry.form = new_concept_word(impl_->rng, impl_->used_forms);
        entry.derivation = "translator-observed-word-form";
        entry.source = "translator-text-intake";
        entry.provenance = "observed-in-user-text;meaning=unverified";
        entry.source_revision = "runtime-text-intake";
        entry.evidence_class = "agent-hypothesis";
        entry.language_status = "realised";
        entry.semantics = "lexical-form;language=de;meaning=unverified";
        entry.introduced_generation = impl_->generation;
        const std::string form = entry.form;
        impl_->add_concept(std::move(entry));
        known = label_to_form.emplace(key, form).first;
        form_to_label.emplace(form, piece);
        ++learned_tokens;
      } else ++dictionary_tokens;
      const std::string& payload = known->second;
      rendered << apply_case_marker(payload, case_marker(piece));
      continue;
    }
    // Known Lunari forms are shown directly. The reserved huhuhu payload is
    // only used for source words absent from the persistent lexicon.
    const std::string marker = case_marker(piece);
    const std::string payload = canonical_concept_key(piece);
    std::optional<std::string> recovered;
    if (const auto known = form_to_label.find(payload); known != form_to_label.end()) recovered = known->second;
    else recovered = unescape_text_token(payload);
    if (!recovered) { rendered << piece; ++unresolved; continue; }
    // The local word list has no auditable English glosses. Preserve the
    // recovered source form rather than inventing an English translation.
    rendered << apply_case_marker(*recovered, marker); ++decoded_tokens;
  }
  std::ostringstream out;
  out << "Lunari text translator\n";
  if (target_language == "constructed") {
    out << "Lunari text:\n" << rendered.str() << "\n-- end Lunari text --\n"
        << "dictionary tokens=" << dictionary_tokens << "\nnewly learned tokens=" << learned_tokens
        << "\nGuarantee: punctuation, whitespace, case markers, dialogue structure, and newly observed UTF-8 tokens are retained without a source-text lookup.";
  } else {
    out << (target_language == "de" ? "Deutsch text:\n" : "English text:\n") << rendered.str() << "\n-- end translated text --\n"
        << "decoded tokens=" << decoded_tokens << "\nunresolved Lunari tokens=" << unresolved;
    if (target_language == "en") out << "\nNote: local German word-list entries have no verified English meanings; recovered German forms are preserved instead of guessed.";
  }
  return out.str();
}
std::string ResearchEngine::concept_communication_trace() const {
  const auto find_label = [this](const std::string& label) -> const ConceptEntry* {
    const auto entry = std::find_if(impl_->concepts.begin(), impl_->concepts.end(), [&label](const ConceptEntry& value) { return value.label == label; });
    return entry == impl_->concepts.end() ? nullptr : &*entry;
  };
  const ConceptEntry* compound = find_label("forest-fire");
  const ConceptEntry* modifier = find_label("danger");
  if (!compound || !modifier) return "Concept communication lab\nRun a generation first; no validated forest-fire/danger family is available.";
  std::map<std::string, std::string> receiver;
  for (const auto& entry : impl_->concepts) if (entry.derivation == "root" || entry.derivation == "imported-root") receiver.emplace(entry.form, entry.label);
  const std::string message = compound->form + " " + modifier->form;
  std::vector<std::string> recovered;
  std::istringstream tokens(message);
  bool decoded = true;
  for (std::string token; tokens >> token;) {
    if (token.find('-') == std::string::npos) {
      const auto known = receiver.find(token); if (known == receiver.end()) { decoded = false; break; } recovered.push_back(known->second);
      continue;
    }
    std::istringstream components(token); std::vector<std::string> labels;
    for (std::string component; std::getline(components, component, '-');) { const auto known = receiver.find(component); if (known == receiver.end()) { decoded = false; break; } labels.push_back(known->second); }
    if (!decoded) break;
    recovered.push_back(join(labels, "-"));
  }
  const std::string expected = "forest-fire danger";
  const std::string result = join(recovered, " ");
  std::ostringstream out;
  out << "Concept communication lab — unseen compositional phrase\n"
      << "Message: " << message << "\n"
      << "Expected concepts: " << expected << "\n"
      << "Receiver reconstruction: " << (decoded ? result : "<decode failure>") << "\n"
      << "Task success: " << ((decoded && result == expected) ? "yes" : "no") << "\n"
      << "Note: the receiver reconstructs basic forms and composes the family label; no CONCEPT_* ID is transmitted.";
  return out.str();
}
std::string ResearchEngine::productive_grammar_trace() const {
  if (impl_->frontier.empty()) return "Productive morphology lab\nNo evolved language candidate available.";
  return productive_grammar_test(impl_->frontier.front());
}
std::string ResearchEngine::phonotactic_trace() const {
  if (impl_->frontier.empty()) return "Lunari phonotactic lab\nNo evolved language candidate available.";
  const auto& language = impl_->frontier.front();
  std::size_t total{}, compact{}, rejected{}, candidate_forms{}, concept_forms{};
  double combined{};
  const auto inspect = [&](const std::string& form) {
    const auto score = phonotactic_score(form); ++total; combined += score.combined;
    if (score.compact_syllables && score.structure > 0.0) ++compact; else ++rejected;
  };
  for (const auto& [_, form] : language.lexicon) { inspect(form); ++candidate_forms; }
  for (const auto& [_, form] : language.morphology) { inspect(form); ++candidate_forms; }
  for (const auto& entry : impl_->concepts) { inspect(entry.form); ++concept_forms; }
  std::ostringstream out;
  out << "Lunari phonotactic lab\n"
      << "phonology version: " << kConceptMorphologyRuleVersion << "\n"
      << "phoneme inventory: vowels a e i o u; onsets m n l r p t k b d g f s h v; codas m n l r s v d\n"
      << "allowed syllables: V, CV, CVC; word formation: compact 1-3 syllable roots with restricted coda-onset pairs\n"
      << "scoring heuristic: C=0.30*structure + 0.30*transition + 0.20*length + 0.20*acoustic-separation\n"
      << "evaluated candidate forms=" << candidate_forms << "\nevaluated persistent concept forms=" << concept_forms << "\nevaluated forms total=" << total << "\ncompact-syllable forms=" << compact << "\nrejected forms=" << rejected
      << "\nmean phonotactic score=" << (total == 0 ? 0.0 : combined / total)
      << "\ncompact-syllable audit: " << (rejected == 0 ? "passed" : "failed")
      << "\nNote: these are explicit research heuristics, not a claim of calibrated human speech data.";
  return out.str();
}
std::string ResearchEngine::neu_lunari_collision_report() const {
  const auto reserved = neu_lunari_reserved_forms();
  std::vector<std::string> collisions;
  for (const auto& form : reserved) if (impl_->used_forms.contains(form)) collisions.push_back(form);
  std::ostringstream out;
  out << "Neu-Lunari function-form collision audit\nlexicon forms inspected=" << impl_->used_forms.size() << "\nreserved forms=" << reserved.size() << "\ncollisions=" << collisions.size();
  if (!collisions.empty()) out << "\ncollision forms=" << join(collisions);
  out << "\nStatus: " << (collisions.empty() ? "clear for proposed reservation" : "reservation blocked; choose replacement forms");
  return out.str();
}
std::string ResearchEngine::neu_lunari_lexicon_report() const {
  std::ostringstream out;
  out << "Neu-Lunari grammar lexicon\nstate format=LENG_STATE 17\nentries=" << impl_->grammar_lexemes.size();
  for (const auto& entry : impl_->grammar_lexemes)
    out << "\n" << entry.concept_id << " | historical=" << entry.historic_form << " | form=" << entry.form << " | version=" << entry.language_version << " | status=" << entry.status << " | pos=" << entry.part_of_speech << " | meaning=" << entry.meaning << " | valency=" << entry.valency << " | scope=" << entry.scope << " | source=" << entry.source << " | note=" << entry.note;
  return out.str();
}
void ResearchEngine::set_neu_lunari_lexicon_status(const std::string& status) {
  if (status != "proposed" && status != "active" && status != "historical") throw std::invalid_argument("Neu-Lunari status must be proposed, active, or historical");
  ensure_neu_lunari_lexemes(impl_->grammar_lexemes);
  for (auto& entry : impl_->grammar_lexemes) {
    entry.status = status;
    entry.note = "status transition reversible; prior and historical forms retained";
  }
}
std::string ResearchEngine::communication_trace() const {
  if (impl_->frontier.empty() || impl_->agents.empty()) return "No trained agents available. Run an experiment first.";
  const Meaning expected{SpeechAct::question, "researcher", "observe", "idea", "if", false, 2, 3};
  const auto& language = impl_->frontier.front();
  const auto encode_started = std::chrono::steady_clock::now();
  const std::string message = encode(language, expected);
  const auto encode_finished = std::chrono::steady_clock::now();
  Meaning decoded;
  const auto& receiver = impl_->agents.front();
  const auto decode_started = std::chrono::steady_clock::now();
  const bool decoded_ok = decode(receiver.private_reverse_lexicon, message, decoded);
  const auto decode_finished = std::chrono::steady_clock::now();
  std::size_t receiver_bytes{};
  for (const auto& [word, atom] : receiver.private_reverse_lexicon) receiver_bytes += word.size() + atom.size();
  const auto encode_us = std::chrono::duration_cast<std::chrono::microseconds>(encode_finished - encode_started).count();
  const auto decode_us = std::chrono::duration_cast<std::chrono::microseconds>(decode_finished - decode_started).count();
  std::ostringstream out;
  out << "Communication lab — sender and receiver use separate lexicon maps\n"
      << "Receiver agent: " << receiver.id << " (learned tokens: " << receiver.private_reverse_lexicon.size() << ", successes: " << receiver.successful_interpretations << ", failures: " << receiver.failed_interpretations << ")\n"
      << "Message: " << message << "\n"
      << "Message bytes: " << message.size() << "\n"
      << "Encode micros: " << encode_us << "\n"
      << "Decode micros: " << decode_us << "\n"
      << "Receiver lexicon payload bytes: " << receiver_bytes << "\n"
      << "Expected semantic frame: " << expected.key() << "\n"
      << "Decoded semantic frame: " << (decoded_ok ? decoded.key() : "<decode failure>") << "\n"
      << "Task success: " << ((decoded_ok && decoded == expected) ? "yes" : "no") << "\n"
      << "Note: the decoder was called with message tokens and its private reverse lexicon only.";
  return out.str();
}
std::string ResearchEngine::evolution_trace() const {
  std::ostringstream out;
  out << "Evolution lab — Pareto archive\n";
  if (impl_->history.empty()) return out.str() + "No completed generations.";
  const std::size_t first = impl_->history.size() > 16 ? impl_->history.size() - 16 : 0;
  for (std::size_t i = first; i < impl_->history.size(); ++i) {
    const auto& candidate = impl_->history[i];
    out << "candidate=" << candidate.id << " parent=" << candidate.parent_id << " success=" << candidate.metrics.communication_success << " semantics=" << candidate.metrics.semantic_accuracy << " bytes=" << candidate.metrics.cost << " pronunciation=" << candidate.metrics.pronounceability << '\n';
  }
  return out.str();
}
std::string ResearchEngine::population_trace() const {
  std::ostringstream out;
  out << "Agent population\n";
  if (impl_->agents.empty()) return out.str() + "No persisted agents.";
  for (const auto& agent : impl_->agents) out << "agent=" << agent.id << " learned_tokens=" << agent.private_reverse_lexicon.size() << " successes=" << agent.successful_interpretations << " failures=" << agent.failed_interpretations << '\n';
  return out.str();
}
HumanExercise ResearchEngine::create_human_exercise() const {
  if (impl_->frontier.empty()) return {"Start an experiment first.", "", 0, false};
  const auto& language = impl_->frontier.front();
  const unsigned lesson = static_cast<unsigned>(impl_->human_attempts % 4);
  if (lesson == 0) return {"Human language lab — Lesson 1 of 4: recognition\n\nThe language word `" + language.lexicon.at("act:request") + "` has which English meaning?\n\nINPUT: enter one English word only.", "request|command", unix_millis(), true};
  if (lesson == 1) return {"Human language lab — Lesson 2 of 4: recall\n\nWhich generated language word means `child`?\n\nINPUT: enter one generated word only.", language.lexicon.at("actor:child"), unix_millis(), false};
  if (lesson == 2) return {"Human language lab — Lesson 3 of 4: short interpretation\n\nDecode this four-word message:\n`" + language.lexicon.at("act:request") + " " + language.lexicon.at("actor:child") + " " + language.lexicon.at("action:build") + " " + language.lexicon.at("object:map") + "`\n\nINPUT: enter exactly four English words: act actor action object.", "request child build map", unix_millis(), true};
  return {"Human language lab — Lesson 4 of 4: short production\n\nForm the message: `request child build map`.\n\nINPUT: enter exactly four generated words in that order. Do not enter field names or semicolons.", language.lexicon.at("act:request") + " " + language.lexicon.at("actor:child") + " " + language.lexicon.at("action:build") + " " + language.lexicon.at("object:map"), unix_millis(), false};
}
bool ResearchEngine::check_human_exercise(const HumanExercise& exercise, const std::string& answer) {
  bool correct = false;
  std::istringstream alternatives(exercise.expected_answer);
  for (std::string alternative; std::getline(alternatives, alternative, '|');) {
    if (canonical_answer(answer) == canonical_answer(alternative)) { correct = true; break; }
  }
  ++impl_->human_attempts;
  if (correct) ++impl_->human_correct;
  if (exercise.interpretation) { ++impl_->human_interpretation_attempts; if (correct) ++impl_->human_interpretation_correct; }
  else { ++impl_->human_production_attempts; if (correct) ++impl_->human_production_correct; }
  const std::uint64_t now = unix_millis();
  if (exercise.issued_unix_ms != 0 && now >= exercise.issued_unix_ms) impl_->human_total_millis += now - exercise.issued_unix_ms;
  return correct;
}
std::string ResearchEngine::human_lab_trace() const {
  std::ostringstream out;
  const double accuracy = impl_->human_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_correct) / impl_->human_attempts;
  const double mean_millis = impl_->human_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_total_millis) / impl_->human_attempts;
  const double recognition = impl_->human_interpretation_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_interpretation_correct) / impl_->human_interpretation_attempts;
  const double production = impl_->human_production_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_production_correct) / impl_->human_production_attempts;
  out << "Human language lab\nattempts=" << impl_->human_attempts << "\ncorrect=" << impl_->human_correct << "\nerror_rate=" << (1.0 - accuracy) << "\nrecognition_rate=" << recognition << "\nproduction_rate=" << production << "\nmean_response_millis=" << mean_millis << "\nNote: these are recorded participant interactions, not predicted human learnability.";
  return out.str();
}
int ResearchEngine::generation() const { return impl_->generation; }

bool self_test(std::string& error) {
  try {
    const std::string multiline_text = "Neun Nächte ritt ich, ohne Himmel über mir.\n"
                                       "Kein Licht in diesen Tälern ... nur Sleipnirs Atem.\n\n"
                                       "Bruder ... ich hab dich gefunden.";
    const auto multiline_lunari = translate_german_lunari_text(multiline_text);
    if (!multiline_lunari.ok || multiline_lunari.text.find("\n\n") == std::string::npos || multiline_lunari.text.find("hami") != std::string::npos || multiline_lunari.text.find("fim loma") != std::string::npos)
      throw std::runtime_error("Neu-Lunari multiline text translation mismatch");
    const auto multiline_german = translate_lunari_german_text(multiline_lunari.text);
    if (!multiline_german.ok || multiline_german.text.find("\n\n") == std::string::npos || multiline_german.text.find("sleipnirs") == std::string::npos)
      throw std::runtime_error("Neu-Lunari multiline text round-trip mismatch");
    const auto compact_word = encode_german_neu_lunari_lexeme("sleipnir");
    if (compact_word.size() != 8 || !assess_neu_lunari_word(compact_word).valid || decode_neu_lunari_german_lexeme(compact_word) != "sleipnir")
      throw std::runtime_error("Neu-Lunari compact lexicon mismatch");
    const auto grammar_spec = parse_lunari_spec("subject=I;verb=tar;object=lun;time=past;aspect=completed;negation=predicate;relation=location;related=dom");
    if (!grammar_spec.ok) throw std::runtime_error("Neu-Lunari specification parser mismatch");
    const auto grammar_sentence = generate_lunari(grammar_spec.clause);
    if (!grammar_sentence.ok || grammar_sentence.text != "mer ren sen nus tar lun nel dom.")
      throw std::runtime_error("Neu-Lunari generator mismatch");
    const auto grammar_recovered = analyse_lunari(grammar_sentence.text);
    if (!grammar_recovered.ok || grammar_recovered.clause.subject != "I" || grammar_recovered.clause.verb != "tar" || grammar_recovered.clause.object != "lun" || grammar_recovered.clause.time != "past" || grammar_recovered.clause.aspect != "completed" || grammar_recovered.clause.negation != "predicate")
      throw std::runtime_error("Neu-Lunari analysis round-trip mismatch");
    if (analyse_lunari("mer.").ok || generate_lunari(LunariClause{}).ok)
      throw std::runtime_error("Neu-Lunari invalid sentence diagnostics mismatch");
    if (analyse_lunari("memekumo tar lun.").diagnostic.find("Historische Form memekumo") == std::string::npos)
      throw std::runtime_error("Neu-Lunari historical-form diagnostic mismatch");
    const auto embedded_spec = parse_lunari_spec("subject=I;verb=tar;embedding=content;embedded={subject=you;verb=lun;time=future}");
    if (!embedded_spec.ok) throw std::runtime_error("Neu-Lunari embedded specification parser mismatch");
    const auto embedded_sentence = generate_lunari(embedded_spec.clause);
    if (!embedded_sentence.ok || embedded_sentence.text != "mer tar ki gav sil lun ku.") throw std::runtime_error("Neu-Lunari embedded generator mismatch");
    const auto embedded_recovered = analyse_lunari(embedded_sentence.text);
    if (!embedded_recovered.ok || embedded_recovered.clause.embedding != "content" || !embedded_recovered.clause.embedded || embedded_recovered.clause.embedded->subject != "you" || embedded_recovered.clause.embedded->time != "future") throw std::runtime_error("Neu-Lunari embedded analysis mismatch");
    const auto nested_spec = parse_lunari_spec("subject=I;verb=tar;embedding=content;embedded={subject=you;verb=lun;embedding=content;embedded={subject=third.singular;verb=tar}}");
    const auto nested_sentence = generate_lunari(nested_spec.clause); const auto nested_recovered = analyse_lunari(nested_sentence.text);
    if (!nested_spec.ok || !nested_sentence.ok || nested_sentence.text != "mer tar ki gav lun ki se tar ku ku." || !nested_recovered.ok || !nested_recovered.clause.embedded || !nested_recovered.clause.embedded->embedded || nested_recovered.clause.embedded->embedded->subject != "third.singular") throw std::runtime_error("Neu-Lunari recursive embedding mismatch");
    const auto linked_spec = parse_lunari_spec("subject=I;verb=tar;connective=and;embedded={subject=you;verb=lun}");
    const auto linked_sentence = generate_lunari(linked_spec.clause); const auto linked_recovered = analyse_lunari(linked_sentence.text);
    if (!linked_spec.ok || !linked_sentence.ok || !linked_recovered.ok || linked_recovered.clause.connective != "and" || !linked_recovered.clause.embedded || linked_recovered.clause.embedded->subject != "you") throw std::runtime_error("Neu-Lunari linkage analysis mismatch");
    const auto quantity_spec = parse_lunari_spec("subject=I;verb=tar;object=lun;quantity=fraction:3/4;measure=vek");
    const auto quantity_sentence = generate_lunari(quantity_spec.clause);
    if (!quantity_spec.ok || !quantity_sentence.ok || quantity_sentence.text != "mer tar fal 3/4 mun vek lun.") throw std::runtime_error("Neu-Lunari quantity generator mismatch");
    const auto quantity_recovered = analyse_lunari(quantity_sentence.text);
    if (!quantity_recovered.ok || quantity_recovered.clause.quantity != "fraction:3/4" || quantity_recovered.clause.measure != "vek") throw std::runtime_error("Neu-Lunari quantity analysis mismatch");
    LunariClause invalid_quantity{}; invalid_quantity.subject = "I"; invalid_quantity.verb = "tar"; invalid_quantity.quantity = "fraction:one/two";
    if (generate_lunari(invalid_quantity).ok) throw std::runtime_error("Neu-Lunari quantity validation mismatch");
    const auto comparison_spec = parse_lunari_spec("subject=I;verb=tar;object=lun;adjective=vaske;degree=more;compared_with=dom");
    const auto comparison_sentence = generate_lunari(comparison_spec.clause);
    if (!comparison_spec.ok || !comparison_sentence.ok || comparison_sentence.text != "mer tar lun mor vaske kom dom.") throw std::runtime_error("Neu-Lunari comparison generator mismatch");
    const auto comparison_recovered = analyse_lunari(comparison_sentence.text);
    if (!comparison_recovered.ok || comparison_recovered.clause.object != "lun" || comparison_recovered.clause.adjective != "vaske" || comparison_recovered.clause.degree != "more" || comparison_recovered.clause.compared_with != "dom") throw std::runtime_error("Neu-Lunari comparison analysis mismatch");
    const auto copula_spec = parse_lunari_spec("subject=lun;verb=copula;predicate_adjective=vaske"); const auto copula_sentence = generate_lunari(copula_spec.clause); const auto copula_recovered = analyse_lunari(copula_sentence.text);
    if (!copula_spec.ok || !copula_sentence.ok || copula_sentence.text != "lun gul vaske." || !copula_recovered.ok || copula_recovered.clause.verb != "copula" || copula_recovered.clause.predicate_adjective != "vaske" || !copula_recovered.clause.object.empty()) throw std::runtime_error("Neu-Lunari copula distinction mismatch");
    const auto stance_spec = parse_lunari_spec("subject=I;verb=tar;probability=probable;emotion=vaske;social=gav;goal=dom;level=report"); const auto stance_sentence = generate_lunari(stance_spec.clause); const auto stance_recovered = analyse_lunari(stance_sentence.text);
    if (!stance_spec.ok || !stance_sentence.ok || stance_sentence.text != "rav pur fel vaske saf gav gol dom mer tar." || !stance_recovered.ok || stance_recovered.clause.probability != "probable" || stance_recovered.clause.emotion != "vaske" || stance_recovered.clause.social_relation != "gav" || stance_recovered.clause.goal != "dom" || stance_recovered.clause.statement_level != "report") throw std::runtime_error("Neu-Lunari statement-level analysis mismatch");
    LunariClause yes_answer{}; yes_answer.speech_act = "answer-yes"; const auto answer_sentence = generate_lunari(yes_answer); const auto answer_recovered = analyse_lunari(answer_sentence.text);
    if (!answer_sentence.ok || answer_sentence.text != "ya." || !answer_recovered.ok || answer_recovered.clause.speech_act != "answer-yes" || !analyse_lunari("na.").ok) throw std::runtime_error("Neu-Lunari short-answer mismatch");
    const auto agent_derivation = derive_neu_lunari_word("tar", "agent"); const auto compound_derivation = derive_neu_lunari_word("luna", "compound", "lun");
    if (!agent_derivation.ok || agent_derivation.form != "tarar" || !compound_derivation.ok || compound_derivation.form != "lunalun" || derive_neu_lunari_word("tar", "compound").ok) throw std::runtime_error("Neu-Lunari word-formation mismatch");
    LunariClause unsupported{}; unsupported.subject = "I"; unsupported.verb = "tar"; unsupported.modality = "telepathy";
    if (generate_lunari(unsupported).ok)
      throw std::runtime_error("Neu-Lunari unsupported-category validation mismatch");
    RunConfig config; config.population = 4; config.generations = 2; config.training_rounds = 50; config.evaluation_rounds = 20;
    ResearchEngine e(config); e.run();
    if (e.generation() != 2 || e.archive().empty()) throw std::runtime_error("evolution did not advance");
    e.set_neu_lunari_lexicon_status("active");
    if (e.neu_lunari_lexicon_report().find("status=active") == std::string::npos) throw std::runtime_error("Neu-Lunari activation mismatch");
    e.set_neu_lunari_lexicon_status("historical");
    if (e.neu_lunari_lexicon_report().find("status=historical") == std::string::npos) throw std::runtime_error("Neu-Lunari rollback mismatch");
    e.set_neu_lunari_lexicon_status("proposed");
    if (e.phonotactic_trace().find("compact-syllable audit: passed") == std::string::npos ||
        e.phonotactic_trace().find("phoneme inventory: vowels a e i o u") == std::string::npos)
      throw std::runtime_error("Lunari phonotactic validation mismatch");
    const double noisy_robustness = e.archive().back().metrics.robustness;
    if (noisy_robustness <= 0.0 || noisy_robustness >= 1.0) throw std::runtime_error("noise robustness metric mismatch");
    if (e.communication_trace().find("Task success: yes") == std::string::npos || e.concept_communication_trace().find("Task success: yes") == std::string::npos || e.evolution_trace().find("candidate=") == std::string::npos) throw std::runtime_error("research laboratory trace mismatch");
    const auto import_path = std::filesystem::temp_directory_path() / "leng-self-import.tsv";
    { std::ofstream fixture(import_path); fixture << "label\tpart_of_speech\tsemantics\trelations\tsource\tprovenance\n" << "rain\tnoun\tweather;water\tcan-affect:forest;causes:flood\tWikidata\tCC0;test-fixture\n" << "flood\tnoun\twater;overflow\t\tWikidata\tCC0;test-fixture\n" << "melody\tnoun\tmusic;ordered-sound\tis-a:music-concept\tWikidata\tCC0;test-fixture\n"; }
    if (e.import_concepts(import_path) != 3) throw std::runtime_error("concept import count mismatch");
    const auto enrichment_path = std::filesystem::temp_directory_path() / "leng-self-enrichment.tsv";
    { std::ofstream fixture(enrichment_path); fixture << "label\tpart_of_speech\tsemantics\trelations\tsource\tprovenance\n" << "tree\tnoun\tliving;plant;external-knowledge\tcan-affect:forest\tWikidata\tCC0;enrichment-fixture\n"; }
    if (e.import_concepts(enrichment_path) != 1) throw std::runtime_error("concept enrichment count mismatch");
    const auto word_list_path = std::filesystem::temp_directory_path() / "leng-self-german-words.txt";
    { std::ofstream fixture(word_list_path); fixture << "Haus\nBett\nHaus\n"; }
    const auto word_progress = e.import_german_word_list_batch(word_list_path, 2);
    if (word_progress.concepts_created != 2 || word_progress.complete ||
        e.translate("Haus", "constructed").find("Constructed language:") == std::string::npos ||
        e.translate("Bett", "constructed").find("Productive morphology:") != std::string::npos)
      throw std::runtime_error("local German word-list import mismatch");
    const auto final_word_progress = e.import_german_word_list_batch(word_list_path, 10);
    if (!final_word_progress.complete || e.lexicon_status().find("local German word forms (meaning unverified)=2") == std::string::npos)
      throw std::runtime_error("local German word-list checkpoint mismatch");
    const std::string lexicon_before_text = e.lexicon_trace();
    if (lexicon_before_text.find("forest-fire") == std::string::npos || lexicon_before_text.find("rain-forest") == std::string::npos || lexicon_before_text.find("rain-flood") == std::string::npos || lexicon_before_text.find("tree-forest") == std::string::npos || lexicon_before_text.find("seeded-human-knowledge+Wikidata") == std::string::npos || lexicon_before_text.find("CC0;test-fixture") == std::string::npos || lexicon_before_text.find("language status: communicatively-validated") == std::string::npos || lexicon_before_text.find("evidence: external-attested") == std::string::npos) throw std::runtime_error("concept lexicon validation mismatch");
    const std::string population_before_save = e.population_trace();
    const auto recognition = e.create_human_exercise(); if (!recognition.interpretation || !e.check_human_exercise(recognition, "request")) throw std::runtime_error("recognition lesson validation mismatch");
    const auto recall = e.create_human_exercise(); if (recall.interpretation || !e.check_human_exercise(recall, recall.expected_answer)) throw std::runtime_error("recall lesson validation mismatch");
    const auto interpretation = e.create_human_exercise(); if (!interpretation.interpretation || !e.check_human_exercise(interpretation, interpretation.expected_answer)) throw std::runtime_error("interpretation lesson validation mismatch");
    const auto production = e.create_human_exercise(); if (production.interpretation || !e.check_human_exercise(production, production.expected_answer) || e.check_human_exercise(production, "incorrect")) throw std::runtime_error("production lesson validation mismatch");
    const std::string german_translation = e.translate("request child build map", "de");
    const std::string constructed_translation = e.translate("sie bauen", "constructed");
    if (german_translation.find("Deutsch: bitte kind bauen karte") == std::string::npos ||
        constructed_translation.find("Constructed language:") == std::string::npos ||
        constructed_translation.find("bauen;pl;present") == std::string::npos ||
        constructed_translation.find("Productive morphology:") == std::string::npos)
      throw std::runtime_error("semantic translator validation mismatch");
    const std::string meaning_forward = e.translate_meaning("Der Mensch bewegt den Stein nicht gestern nahe.", "constructed");
    const auto frame_start = meaning_forward.find("Lunari frame:\n");
    const auto frame_end = meaning_forward.find("\n-- end Lunari frame --", frame_start);
    if (frame_start == std::string::npos || frame_end == std::string::npos) throw std::runtime_error("semantic frame encoding mismatch");
    const std::string meaning_frame = meaning_forward.substr(frame_start + std::string("Lunari frame:\n").size(), frame_end - (frame_start + std::string("Lunari frame:\n").size()));
    const std::string meaning_backward = e.translate_meaning(meaning_frame, "en");
    if (meaning_backward.find("human / move / stone") == std::string::npos ||
        meaning_backward.find("Negation: yes") == std::string::npos ||
        meaning_backward.find("Time: past") == std::string::npos ||
        meaning_backward.find("Decoded independently") == std::string::npos)
      throw std::runtime_error("independent semantic meaning channel mismatch");
    const auto semantic_state_path = std::filesystem::temp_directory_path() / "leng-semantic-channel.leng";
    e.save(semantic_state_path);
    ResearchEngine independently_loaded;
    independently_loaded.load(semantic_state_path);
    const std::string persisted_meaning = independently_loaded.translate_meaning(meaning_frame, "de");
    if (persisted_meaning.find("mensch / bewegen / stein") == std::string::npos ||
        persisted_meaning.find("Negation: ja") == std::string::npos ||
        persisted_meaning.find("Zeit: Vergangenheit") == std::string::npos)
      throw std::runtime_error("persisted independent semantic channel mismatch");
    const std::string text_fixture = "\xE2\x80\x9E" "Bett?" "\xE2\x80\x9C" ", fragte Sie.\n\nQzx-17 bleibt!";
    const auto extract_text = [](const std::string& report, const std::string& prefix, const std::string& footer) {
      const auto start = report.find(prefix); if (start == std::string::npos) return std::string{};
      const auto value_start = start + prefix.size(); const auto end = report.find(footer, value_start);
      return report.substr(value_start, end == std::string::npos ? std::string::npos : end - value_start);
    };
    const std::string lunari_text = extract_text(e.translate_text(text_fixture, "constructed"), "Lunari text:\n", "\n-- end Lunari text --");
    const std::string text_round_trip = extract_text(e.translate_text(lunari_text, "de"), "Deutsch text:\n", "\n-- end translated text --");
    if (lunari_text.empty() || lunari_text.find("huhuhu") != std::string::npos || text_round_trip != text_fixture) throw std::runtime_error("lossless Lunari text translation mismatch");
    if (e.productive_grammar_trace().find("regular-marker validation: passed") == std::string::npos ||
        e.productive_grammar_trace().find("independent receiver reconstruction: lernen;pl;past") == std::string::npos ||
        e.translate("sie lernen", "constructed").find("lernen;pl;present") == std::string::npos ||
        e.translate("sie lernten", "constructed").find("lernen;pl;past") == std::string::npos)
      throw std::runtime_error("productive morphology validation mismatch");
    if (e.dictionary_page("rain", 0).find("English: rain") == std::string::npos ||
        e.dictionary_page("rain", 0).find("Deutsch: regen") == std::string::npos)
      throw std::runtime_error("dictionary validation mismatch");
    const std::string lexicon_before_save = e.lexicon_trace();
    const auto path = std::filesystem::temp_directory_path() / "leng-self-test.state"; const auto export_dir = std::filesystem::temp_directory_path() / "leng-self-export";
    e.save(path); e.export_results(export_dir);
    if (!std::filesystem::exists(export_dir / "manifest.txt") || !std::filesystem::exists(export_dir / "pareto_archive.csv") || !std::filesystem::exists(export_dir / "evolutionary_lexicon.txt") || e.human_lab_trace().find("recognition_rate=") == std::string::npos) throw std::runtime_error("research export or human metrics mismatch");
    std::string exported_lexicon_text;
    { std::ifstream exported_lexicon(export_dir / "evolutionary_lexicon.txt"); std::ostringstream content; content << exported_lexicon.rdbuf(); exported_lexicon_text = content.str(); }
    if (exported_lexicon_text.find("rain-flood") == std::string::npos || exported_lexicon_text.find("CC0;test-fixture") == std::string::npos) throw std::runtime_error("exported concept provenance mismatch");
    if (compare_research_states(path, path).find("Generation delta (right-left): 0") == std::string::npos) throw std::runtime_error("state comparison mismatch");
    RunConfig resume_config = config; resume_config.generations = 1; ResearchEngine restored(resume_config); ResearchEngine replay(resume_config); restored.load(path); replay.load(path);
    if (restored.generation() != e.generation() || restored.export_language() != e.export_language() || restored.population_trace() != population_before_save || restored.lexicon_trace() != lexicon_before_save || restored.productive_grammar_trace() != e.productive_grammar_trace() || restored.neu_lunari_lexicon_report().find("grammar:relation:material") == std::string::npos || restored.neu_lunari_lexicon_report().find("grammar:pronoun:first-singular") == std::string::npos || restored.neu_lunari_lexicon_report().find("grammar:embedding:quote-close") == std::string::npos || restored.neu_lunari_lexicon_report().find("valency=two clauses or embedded clause") == std::string::npos) throw std::runtime_error("persistence round-trip mismatch");
    restored.run(); replay.run();
    if (restored.generation() != 3 || restored.export_language() != replay.export_language() || restored.population_trace() != replay.population_trace() || restored.lexicon_trace() != replay.lexicon_trace()) throw std::runtime_error("deterministic continuation mismatch");
    std::filesystem::remove(path); std::filesystem::remove(import_path); std::filesystem::remove(enrichment_path); std::filesystem::remove(word_list_path); std::filesystem::remove_all(export_dir); const auto parity = run_reward_parity_test(); if (parity.attempted && !parity.passed) throw std::runtime_error(parity.detail); return true;
  } catch (const std::exception& ex) { error = ex.what(); return false; }
}
std::string compare_research_states(const std::filesystem::path& left, const std::filesystem::path& right) {
  ResearchEngine first; first.load(left);
  ResearchEngine second; second.load(right);
  const auto describe = [](const ResearchEngine& engine, const std::filesystem::path& path) {
    std::ostringstream out;
    out << path.string() << "\nGeneration: " << engine.generation();
    if (!engine.archive().empty()) {
      const auto& candidate = engine.archive().back();
      out << "\nCandidate: " << candidate.id << "\nCommunication: " << candidate.metrics.communication_success
          << "\nSemantics: " << candidate.metrics.semantic_accuracy << "\nBytes/message: " << candidate.metrics.cost
          << "\nPronounceability: " << candidate.metrics.pronounceability << "\nRobustness: " << candidate.metrics.robustness;
    }
    return out.str();
  };
  std::ostringstream out;
  out << "Experiment comparison (persisted engine states)\n\n" << describe(first, left) << "\n\n" << describe(second, right)
      << "\n\nGeneration delta (right-left): " << (second.generation() - first.generation());
  if (!first.archive().empty() && !second.archive().empty()) {
    const auto& a = first.archive().back().metrics; const auto& b = second.archive().back().metrics;
    out << "\nCommunication delta: " << (b.communication_success - a.communication_success)
        << "\nSemantic delta: " << (b.semantic_accuracy - a.semantic_accuracy)
        << "\nCost delta: " << (b.cost - a.cost) << "\nRobustness delta: " << (b.robustness - a.robustness);
  }
  return out.str();
}
}  // namespace leng
