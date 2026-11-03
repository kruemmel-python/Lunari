#include "leng/lunari_grammar.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <vector>

namespace leng {
namespace {
constexpr const char* kVersion = "neu-lunari-v1-proposed";
const std::map<std::string, std::string> kForms{
    {"I", "mer"}, {"you", "gav"}, {"they", "sel"}, {"third.singular", "se"}, {"you.plural", "gal"}, {"we.inclusive", "rin"}, {"we.exclusive", "lan"}, {"reflexive", "sir"}, {"reciprocal", "san"},
    {"question", "ke"}, {"request", "va"}, {"prohibition", "vas"}, {"answer-yes", "ya"}, {"answer-no", "na"}, {"past", "ren"}, {"future", "sil"},
    {"progressive", "lir"}, {"completed", "sen"}, {"habitual", "tan"}, {"predicate", "nus"}, {"statement", "nor"},
    {"passive", "vel"}, {"definite", "tel"}, {"indefinite", "sar"}, {"plural", "lin"}, {"ability", "kan"},
    {"about", "mar"}, {"atleast", "las"}, {"atmost", "sav"}, {"fraction", "fal"}, {"ordinal", "tor"}, {"measure", "mun"},
    {"very", "daz"}, {"more", "mor"}, {"less", "les"}, {"equal", "sim"}, {"most", "mas"}, {"comparison", "kom"}, {"copula", "gul"},
    {"possibility", "ves"}, {"permission", "dar"}, {"obligation", "mur"}, {"recommendation", "res"}, {"intent", "vin"}, {"wish", "lum"},
    {"location", "nel"}, {"destination", "tal"}, {"source", "sor"}, {"recipient", "rem"}, {"instrument", "ket"}, {"companion", "val"},
    {"topic", "tem"}, {"purpose", "sur"}, {"possession", "bel"}, {"part", "pel"}, {"content", "mir"}, {"material", "dam"},
    {"person", "man"}, {"thing", "min"}, {"place", "lon"}, {"time", "tir"}, {"reason", "kar"}, {"manner", "sev"}, {"amount", "num"}, {"choice", "var"},
    {"subject-focus", "sam"}, {"object-focus", "som"}, {"modality-focus", "nam"},
    {"probable", "pur"}, {"uncertain", "sul"}, {"report", "rav"}, {"hypothesis", "hes"}, {"fiction", "fik"},
    {"emotion-marker", "fel"}, {"social-marker", "saf"}, {"goal-marker", "gol"},
    {"and", "ran"}, {"or", "lor"}, {"but", "bas"}, {"cause", "kos"}, {"condition", "kim"}, {"concession", "lus"}, {"purpose-clause", "suv"}, {"temporal", "tim"},
    {"content-open", "ki"}, {"content-close", "ku"}, {"relative-open", "ril"}, {"relative-close", "rul"}, {"quote-open", "qi"}, {"quote-close", "qu"},
    {"text-subject", "fim"}, {"text-mode", "loma"},
};
const std::map<std::string, std::string> kHistoricalAliases{
    {"memekumo", "mer"}, {"gavogedu", "gav"}, {"vipakevo", "vaske"},
};
// New, versioned content lexicon.  It deliberately does not reuse the
// earlier Alta-Lunari inventory.  Remaining German content words are encoded
// losslessly below, so the end-user translator never falls back to it.
const std::map<std::string, std::string> kContentForms{
    {"today", "tiri"}, {"yesterday", "yara"}, {"tomorrow", "sila"},
    {"monday", "mona"}, {"tuesday", "tusar"}, {"wednesday", "vemar"}, {"thursday", "donar"}, {"friday", "farim"}, {"saturday", "savar"}, {"sunday", "sunar"},
    {"human", "manu"}, {"machine", "mekar"}, {"child", "kinar"}, {"researcher", "resar"}, {"agent", "gamar"},
    {"stone", "sotar"}, {"signal", "sinar"}, {"map", "mapa"}, {"water", "valar"}, {"idea", "idar"},
    {"see", "sivar"}, {"move", "mova"}, {"give", "giva"}, {"observe", "sava"}, {"build", "bura"}, {"ask", "kasa"},
    {"good", "luma"}, {"correct", "sura"}, {"ready", "rava"}, {"true", "tura"}, {"false", "fala"},
};
const std::map<std::string, std::string> kGermanMeanings{
    {"heute", "today"}, {"gestern", "yesterday"}, {"morgen", "tomorrow"},
    {"montag", "monday"}, {"dienstag", "tuesday"}, {"mittwoch", "wednesday"}, {"donnerstag", "thursday"}, {"freitag", "friday"}, {"samstag", "saturday"}, {"sonntag", "sunday"},
    {"mensch", "human"}, {"maschine", "machine"}, {"kind", "child"}, {"forscher", "researcher"}, {"agent", "agent"},
    {"stein", "stone"}, {"signal", "signal"}, {"karte", "map"}, {"wasser", "water"}, {"idee", "idea"},
    {"gut", "good"}, {"richtig", "correct"}, {"bereit", "ready"}, {"wahr", "true"}, {"falsch", "false"},
};
const std::map<char, std::string> kGermanCode{
    {'a', "ba"}, {'b', "be"}, {'c', "bi"}, {'d', "bo"}, {'e', "bu"},
    {'f', "da"}, {'g', "de"}, {'h', "di"}, {'i', "do"}, {'j', "du"},
    {'k', "fa"}, {'l', "fe"}, {'m', "fi"}, {'n', "fo"}, {'o', "fu"},
    {'p', "ga"}, {'q', "ge"}, {'r', "gi"}, {'s', "go"}, {'t', "gu"},
    {'u', "ka"}, {'v', "ke"}, {'w', "ki"}, {'x', "ko"}, {'y', "ku"}, {'z', "la"},
};
std::map<std::string, std::string> learned_german_forms;
std::map<std::string, std::string> learned_form_german;
std::mutex learned_lexicon_mutex;
std::string trim(std::string value) { const auto b = std::find_if_not(value.begin(), value.end(), [](unsigned char c){ return std::isspace(c); }); const auto e = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c){ return std::isspace(c); }).base(); return b >= e ? "" : std::string(b, e); }
std::vector<std::string> split(const std::string& value, char separator) { std::vector<std::string> values; std::istringstream in(value); for (std::string part; std::getline(in, part, separator);) values.push_back(trim(part)); return values; }
std::vector<std::string> split_top_level(const std::string& value, char separator) { std::vector<std::string> values; std::string current; int depth{}; for (const char ch : value) { if (ch == '{') ++depth; else if (ch == '}' && depth > 0) --depth; if (ch == separator && depth == 0) { values.push_back(trim(current)); current.clear(); } else current.push_back(ch); } values.push_back(trim(current)); return values; }
std::string form(const std::string& value) { const auto it = kForms.find(value); if (it != kForms.end()) return it->second; const auto content = kContentForms.find(value); return content == kContentForms.end() ? value : content->second; }
std::string key_for(const std::string& value) { for (const auto& [key, surface] : kForms) if (surface == value) return key; return {}; }
std::string lower_german(std::string value) {
  const std::array<std::pair<std::string, std::string>, 4> replacements{{{"ä", "ae"}, {"ö", "oe"}, {"ü", "ue"}, {"ß", "ss"}}};
  for (const auto& [from, to] : replacements) { std::size_t pos{}; while ((pos = value.find(from, pos)) != std::string::npos) { value.replace(pos, from.size(), to); pos += to.size(); } }
  for (char& character : value) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  return value;
}
std::uint64_t word_hash(const std::string& word, std::uint64_t salt = 0) {
  std::uint64_t hash = 1469598103934665603ULL ^ salt;
  for (const unsigned char character : word) { hash ^= character; hash *= 1099511628211ULL; }
  return hash;
}
std::string compact_lunari_form(std::uint64_t hash) {
  static constexpr std::array<const char*, 14> onsets{{"m", "n", "l", "r", "p", "t", "k", "b", "d", "g", "f", "s", "v", "h"}};
  static constexpr std::array<const char*, 5> vowels{{"a", "e", "i", "o", "u"}};
  std::string form;
  for (int syllable = 0; syllable < 4; ++syllable) { form += onsets[hash % onsets.size()]; hash /= onsets.size(); form += vowels[hash % vowels.size()]; hash /= vowels.size(); }
  return form;
}
std::string encode_german_lexeme(const std::string& raw) {
  const auto word = lower_german(raw); const auto meaning = kGermanMeanings.find(word); if (meaning != kGermanMeanings.end()) return kContentForms.at(meaning->second); const auto known = kContentForms.find(word); if (known != kContentForms.end()) return known->second;
  if (word.empty() || !std::all_of(word.begin(), word.end(), [](unsigned char character) { return character >= 'a' && character <= 'z'; })) return {};
  std::lock_guard lock(learned_lexicon_mutex);
  if (const auto existing = learned_german_forms.find(word); existing != learned_german_forms.end()) return existing->second;
  for (std::uint64_t salt{};; ++salt) {
    const std::string candidate = compact_lunari_form(word_hash(word, salt));
    const bool content_collision = std::any_of(kContentForms.begin(), kContentForms.end(), [&candidate](const auto& entry) { return entry.second == candidate; });
    if (key_for(candidate).empty() && !content_collision && !learned_form_german.contains(candidate)) {
      learned_german_forms.emplace(word, candidate); learned_form_german.emplace(candidate, word); return candidate;
    }
  }
}
std::string content_key_for(const std::string& value) { for (const auto& [meaning, surface] : kContentForms) if (surface == value) return meaning; return value; }
std::string german_label(const std::string& value) { for (const auto& [german, meaning] : kGermanMeanings) if (meaning == value) return german; return value; }
std::string decode_german_lexeme(const std::string& value) {
  for (const auto& [meaning, surface] : kContentForms) if (surface == value) return german_label(meaning);
  { std::lock_guard lock(learned_lexicon_mutex); if (const auto learned = learned_form_german.find(value); learned != learned_form_german.end()) return learned->second; }
  if (value.rfind("hami", 0) != 0 || (value.size() - 4) % 2 != 0) return value;
  std::string decoded; for (std::size_t i = 4; i < value.size(); i += 2) { const auto pair = value.substr(i, 2); const auto found = std::find_if(kGermanCode.begin(), kGermanCode.end(), [&pair](const auto& entry) { return entry.second == pair; }); if (found == kGermanCode.end()) return value; decoded.push_back(found->first); } return decoded;
}
bool valid(const std::string& value, const std::initializer_list<const char*> choices) { return value.empty() || std::find_if(choices.begin(), choices.end(), [&value](const char* choice){ return value == choice; }) != choices.end(); }
bool digits(const std::string& value) { return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isdigit(ch); }); }
bool valid_quantity(const std::string& value) { if (value.empty() || value == "plural") return true; const auto separator = value.find(':'); if (separator == std::string::npos) return digits(value); const auto kind = value.substr(0, separator), amount = value.substr(separator + 1); if (kind == "exact" || kind == "about" || kind == "atleast" || kind == "atmost" || kind == "ordinal") return digits(amount); if (kind == "fraction") { const auto slash = amount.find('/'); return slash != std::string::npos && digits(amount.substr(0, slash)) && digits(amount.substr(slash + 1)); } return false; }
void append_quantity(std::vector<std::string>& words, const std::string& value) { if (value == "plural") { words.push_back(form("plural")); return; } const auto separator = value.find(':'); if (separator == std::string::npos) { words.push_back(value); return; } const auto kind = value.substr(0, separator), amount = value.substr(separator + 1); if (kind != "exact") words.push_back(form(kind)); words.push_back(amount); }
bool vowel(char c) { return std::string{"aeiou"}.find(c) != std::string::npos; }
bool onset(char c) { return std::string{"mnlrptkbdgfshv"}.find(c) != std::string::npos; }
bool coda(char c) { return std::string{"mnlrsvd"}.find(c) != std::string::npos; }
bool join(char left, char right) { return std::string{"mpmtmkmvnsntnknvlklvltrtrkrnrlmlvsmstskvrdrdv"}.find(std::string{left} + right) != std::string::npos; }
}

LunariGrammarResult parse_lunari_spec(const std::string& specification) {
  LunariGrammarResult result; LunariClause clause;
  for (const auto& piece : split_top_level(specification, ';')) {
    const auto pos = piece.find('='); if (pos == std::string::npos) { result.diagnostic = "Ungültige Bedeutungseingabe: erwartet schlüssel=wert"; return result; }
    const std::string key = trim(piece.substr(0, pos)), value = trim(piece.substr(pos + 1));
    if (key == "act") clause.speech_act = value; else if (key == "subject") clause.subject = value; else if (key == "verb") clause.verb = value; else if (key == "object") clause.object = value; else if (key == "determiner") clause.determiner = value; else if (key == "quantity") clause.quantity = value; else if (key == "measure") clause.measure = value; else if (key == "adjective") clause.adjective = value; else if (key == "predicate_adjective") clause.predicate_adjective = value; else if (key == "degree") clause.degree = value; else if (key == "compared_with") clause.compared_with = value; else if (key == "time") clause.time = value; else if (key == "aspect") clause.aspect = value; else if (key == "modality") clause.modality = value; else if (key == "negation") clause.negation = value; else if (key == "relation") clause.relation = value; else if (key == "related") clause.related = value; else if (key == "question") clause.question_word = value; else if (key == "focus_negation") clause.focus_negation = value; else if (key == "probability") clause.probability = value; else if (key == "emotion") clause.emotion = value; else if (key == "social") clause.social_relation = value; else if (key == "goal") clause.goal = value; else if (key == "level") clause.statement_level = value; else if (key == "connective") clause.connective = value; else if (key == "embedding") clause.embedding = value; else if (key == "embedded") { if (value.size() < 2 || value.front() != '{' || value.back() != '}') { result.diagnostic = "embedded erwartet {schlüssel=wert;...}"; return result; } const auto nested = parse_lunari_spec(value.substr(1, value.size() - 2)); if (!nested.ok) { result.diagnostic = "Eingebetteter Satz: " + nested.diagnostic; return result; } clause.embedded = std::make_shared<LunariClause>(nested.clause); } else if (key == "passive") clause.passive = value == "true"; else { result.diagnostic = "Unbekanntes Bedeutungsfeld: " + key; return result; }
  }
  result.ok = true; result.clause = clause; return result;
}

LunariGrammarResult parse_german_lunari_sentence(const std::string& sentence) {
  LunariGrammarResult result; LunariClause clause;
  const bool question = sentence.find('?') != std::string::npos;
  std::string cleaned = lower_german(sentence);
  for (char& character : cleaned) if (!std::isalpha(static_cast<unsigned char>(character)) && character != ' ') character = ' ';
  auto tokens = split(cleaned, ' ');
  tokens.erase(std::remove(tokens.begin(), tokens.end(), ""), tokens.end());
  if (tokens.empty()) { result.diagnostic = "Leere deutsche Eingabe."; return result; }
  if (tokens.size() == 1 && (tokens.front() == "ja" || tokens.front() == "nein")) { clause.speech_act = tokens.front() == "ja" ? "answer-yes" : "answer-no"; result.ok = true; result.clause = clause; return result; }
  if (tokens.front() == "ja" || tokens.front() == "nein") tokens.erase(tokens.begin());
  const std::map<std::string, std::string> pronouns{{"ich", "I"}, {"du", "you"}, {"er", "third.singular"}, {"sie", "third.singular"}, {"es", "third.singular"}, {"wir", "we.inclusive"}, {"ihr", "you.plural"}};
  const std::map<std::string, std::string> verbs{{"sehe", "see"}, {"sehen", "see"}, {"sieht", "see"}, {"sah", "see"}, {"bewege", "move"}, {"bewegen", "move"}, {"bewegt", "move"}, {"gab", "give"}, {"gebe", "give"}, {"geben", "give"}, {"gibt", "give"}, {"beobachte", "observe"}, {"beobachten", "observe"}, {"beobachtet", "observe"}, {"baue", "build"}, {"bauen", "build"}, {"baut", "build"}, {"frage", "ask"}, {"fragen", "ask"}, {"fragt", "ask"}};
  const auto ignored = [](const std::string& token) { return token == "der" || token == "die" || token == "das" || token == "den" || token == "dem" || token == "ein" || token == "eine" || token == "einen" || token == "und"; };
  const auto copula = [](const std::string& token) { return token == "bin" || token == "bist" || token == "ist" || token == "sind" || token == "seid" || token == "war" || token == "waren"; };
  const auto text_mode = [&]() {
    clause.speech_act = question ? "question" : "statement";
    if (std::find(tokens.begin(), tokens.end(), "nicht") != tokens.end()) clause.negation = "predicate";
    if (std::find(tokens.begin(), tokens.end(), "gestern") != tokens.end()) clause.time = "past";
    if (std::find(tokens.begin(), tokens.end(), "morgen") != tokens.end()) clause.time = "future";
    std::size_t subject_index{};
    for (std::size_t index = 0; index < tokens.size(); ++index) if (pronouns.contains(tokens[index])) { subject_index = index; break; }
    const auto subject = tokens[subject_index];
    clause.subject = pronouns.contains(subject) ? pronouns.at(subject) : encode_german_lexeme(subject);
    std::size_t verb_index = subject_index == 0 ? 1 : (subject_index >= 2 ? subject_index - 1 : subject_index + 1);
    if (verb_index >= tokens.size()) verb_index = subject_index == 0 ? subject_index : 0;
    clause.verb = encode_german_lexeme(tokens[verb_index]);
    for (std::size_t index = 0; index < tokens.size(); ++index) {
      if (index == subject_index || index == verb_index || tokens[index] == "nicht" || tokens[index] == "gestern" || tokens[index] == "morgen") continue;
      const auto form = encode_german_lexeme(tokens[index]); if (form.empty()) { result.diagnostic = "Nicht kodierbares deutsches Wort: " + tokens[index]; return false; }
      if (!clause.object.empty()) clause.object += ' ';
      clause.object += form;
    }
    result.ok = !clause.subject.empty() && !clause.verb.empty(); result.clause = clause; if (!result.ok) result.diagnostic = "Leere deutsche Eingabe."; return result.ok;
  };
  for (const auto& token : tokens) {
    const bool known = pronouns.contains(token) || verbs.contains(token) || ignored(token) || copula(token) || token == "nicht" || token == "gestern" || token == "morgen" || token == "ja" || token == "nein" || kGermanMeanings.contains(token);
    if (!known) { text_mode(); return result; }
  }
  std::size_t verb_index = tokens.size();
  for (std::size_t index = 0; index < tokens.size(); ++index) if (verbs.contains(tokens[index]) || copula(tokens[index])) { verb_index = index; break; }
  if (verb_index == tokens.size()) { text_mode(); return result; }
  clause.speech_act = question ? "question" : "statement";
  if (std::find(tokens.begin(), tokens.end(), "nicht") != tokens.end()) clause.negation = "predicate";
  if (std::find(tokens.begin(), tokens.end(), "gestern") != tokens.end()) clause.time = "past";
  if (std::find(tokens.begin(), tokens.end(), "morgen") != tokens.end()) clause.time = "future";
  const auto lexical = [](const std::string& token) { return !token.empty() && token != "nicht" && token != "gestern" && token != "morgen"; };
  const auto assign_subject = [&](const std::string& token) { if (const auto found = pronouns.find(token); found != pronouns.end()) clause.subject = found->second; else clause.subject = encode_german_lexeme(token); };
  std::size_t subject_index = tokens.size();
  if (verb_index > 0) {
    for (std::size_t index = 0; index < verb_index; ++index) if (!ignored(tokens[index]) && lexical(tokens[index])) { assign_subject(tokens[index]); subject_index = index; break; }
  } else {
    for (std::size_t index = 1; index < tokens.size(); ++index) if (!ignored(tokens[index]) && lexical(tokens[index])) { assign_subject(tokens[index]); subject_index = index; break; }
  }
  if (clause.subject.empty()) { result.diagnostic = "Neu-Lunari konnte kein deutsches Subjekt erkennen."; return result; }
  if (copula(tokens[verb_index])) {
    clause.verb = "copula";
    for (std::size_t index = std::max(verb_index + 1, subject_index + 1); index < tokens.size(); ++index) if (!ignored(tokens[index]) && lexical(tokens[index])) { clause.predicate_adjective = encode_german_lexeme(tokens[index]); break; }
    if (clause.predicate_adjective.empty()) { result.diagnostic = "Die deutsche Kopula benötigt eine Eigenschaft, z. B. ‚Heute ist Freitag.‘"; return result; }
  } else {
    clause.verb = verbs.at(tokens[verb_index]);
    for (std::size_t index = verb_index + 1; index < tokens.size(); ++index) if (!ignored(tokens[index]) && lexical(tokens[index])) { clause.object = encode_german_lexeme(tokens[index]); break; }
  }
  result.ok = true; result.clause = clause; return result;
}

std::string render_lunari_clause_german(const LunariClause& clause) {
  if (clause.speech_act == "answer-yes") return "Ja.";
  if (clause.speech_act == "answer-no") return "Nein.";
  if (clause.subject == "text-subject" && clause.verb == "text-mode") {
    std::istringstream words(clause.object); std::ostringstream output;
    for (std::string word; words >> word;) { if (output.tellp() > 0) output << ' '; output << decode_german_lexeme(word); }
    std::string text = output.str(); if (!text.empty()) text.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(text.front()))); text += clause.speech_act == "question" ? '?' : '.'; return text;
  }
  const std::map<std::string, std::string> pronouns{{"I", "ich"}, {"you", "du"}, {"third.singular", "es"}, {"you.plural", "ihr"}, {"we.inclusive", "wir"}, {"we.exclusive", "wir"}, {"they", "sie"}};
  const std::map<std::string, std::string> verbs{{"see", "sieht"}, {"move", "bewegt"}, {"give", "gibt"}, {"observe", "beobachtet"}, {"build", "baut"}, {"ask", "fragt"}};
  const auto german = [&](const std::string& value) { const auto pronoun = pronouns.find(value); return pronoun == pronouns.end() ? decode_german_lexeme(value) : pronoun->second; };
  const auto german_phrase = [&](const std::string& value) { std::istringstream words(value); std::ostringstream output; for (std::string word; words >> word;) { if (output.tellp() > 0) output << ' '; output << german(word); } return output.str(); };
  const std::string subject = german_phrase(clause.subject);
  const std::string verb_key = content_key_for(clause.verb);
  std::string predicate = verb_key == "copula" ? "ist" : (verbs.contains(verb_key) ? verbs.at(verb_key) : decode_german_lexeme(clause.verb));
  std::string complement = clause.verb == "copula" ? german_phrase(clause.predicate_adjective) : german_phrase(clause.object);
  if (clause.negation == "predicate") predicate += " nicht";
  std::ostringstream output;
  if (clause.time == "past") output << "Gestern "; else if (clause.time == "future") output << "Morgen ";
  if (clause.speech_act == "question") output << predicate << ' ' << subject;
  else output << subject << ' ' << predicate;
  if (!complement.empty()) output << ' ' << complement;
  output << (clause.speech_act == "question" ? '?' : '.');
  std::string text = output.str(); if (!text.empty()) text.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(text.front()))); return text;
}

LunariTextResult translate_german_lunari_text(const std::string& text) {
  LunariTextResult result; std::string sentence; std::size_t sentence_number{};
  const auto append_sentence = [&](const std::string& source) -> bool {
    if (std::none_of(source.begin(), source.end(), [](unsigned char character) { return std::isalpha(character); })) return true;
    ++sentence_number;
    const auto parsed = parse_german_lunari_sentence(source);
    if (!parsed.ok) { result.diagnostic = "Satz " + std::to_string(sentence_number) + ": " + parsed.diagnostic; return false; }
    const auto generated = generate_lunari(parsed.clause);
    if (!generated.ok) { result.diagnostic = "Satz " + std::to_string(sentence_number) + ": " + generated.diagnostic; return false; }
    result.text += generated.text;
    return true;
  };
  for (std::size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    const bool ellipsis = character == '.' && ((index > 0 && text[index - 1] == '.') || (index + 1 < text.size() && text[index + 1] == '.'));
    if ((character == '.' && !ellipsis) || character == '!' || character == '?') {
      sentence += character;
      if (!append_sentence(sentence)) return result;
      sentence.clear();
    } else if (character == '\r' || character == '\n') {
      if (!append_sentence(sentence)) return result;
      sentence.clear(); result.text += character;
    } else if (sentence.empty() && std::isspace(static_cast<unsigned char>(character))) {
      result.text += character;
    } else {
      sentence += character;
    }
  }
  if (!append_sentence(sentence)) return result;
  result.ok = !result.text.empty(); if (!result.ok) result.diagnostic = "Leere deutsche Eingabe."; return result;
}

LunariTextResult translate_lunari_german_text(const std::string& text) {
  LunariTextResult result; std::string sentence; std::size_t sentence_number{};
  const auto append_sentence = [&](const std::string& source) -> bool {
    if (std::none_of(source.begin(), source.end(), [](unsigned char character) { return std::isalpha(character); })) return true;
    ++sentence_number;
    const auto analysed = analyse_lunari(source);
    if (!analysed.ok) { result.diagnostic = "Satz " + std::to_string(sentence_number) + ": " + analysed.diagnostic; return false; }
    result.text += render_lunari_clause_german(analysed.clause);
    return true;
  };
  for (std::size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    const bool ellipsis = character == '.' && ((index > 0 && text[index - 1] == '.') || (index + 1 < text.size() && text[index + 1] == '.'));
    if ((character == '.' && !ellipsis) || character == '!' || character == '?') {
      sentence += character;
      if (!append_sentence(sentence)) return result;
      sentence.clear();
    } else if (character == '\r' || character == '\n') {
      if (!append_sentence(sentence)) return result;
      sentence.clear(); result.text += character;
    } else if (sentence.empty() && std::isspace(static_cast<unsigned char>(character))) {
      result.text += character;
    } else {
      sentence += character;
    }
  }
  if (!append_sentence(sentence)) return result;
  result.ok = !result.text.empty(); if (!result.ok) result.diagnostic = "Leere Lunari-Eingabe."; return result;
}

std::string encode_german_neu_lunari_lexeme(const std::string& word) { return encode_german_lexeme(word); }
std::string decode_neu_lunari_german_lexeme(const std::string& word) { return decode_german_lexeme(word); }
void load_neu_lunari_lexicon(const std::filesystem::path& path) {
  std::ifstream input(path); if (!input) return;
  std::string header; if (!std::getline(input, header) || header.find("Neu-Lunari v2") == std::string::npos) return;
  std::lock_guard lock(learned_lexicon_mutex); learned_german_forms.clear(); learned_form_german.clear();
  std::string line; std::getline(input, line); // column header
  while (std::getline(input, line)) {
    const auto separator = line.find('\t'); if (separator == std::string::npos) continue;
    const auto german = lower_german(line.substr(0, separator)); const auto form = trim(line.substr(separator + 1));
    if (german.empty() || form.empty() || learned_form_german.contains(form)) continue;
    learned_german_forms.emplace(german, form); learned_form_german.emplace(form, german);
  }
}

LunariGrammarResult generate_lunari(const LunariClause& c) {
  LunariGrammarResult result; result.clause = c;
  if (c.speech_act == "answer-yes" || c.speech_act == "answer-no") { result.ok = true; result.text = form(c.speech_act) + "."; return result; }
  if (c.subject.empty() || c.verb.empty()) { result.diagnostic = "Unvollständiger Satz: subject und verb sind erforderlich."; return result; }
  if (!valid(c.speech_act, {"statement", "question", "request", "prohibition"}) || !valid(c.time, {"present", "past", "future"}) || !valid(c.aspect, {"plain", "progressive", "completed", "habitual"}) || !valid(c.negation, {"none", "predicate", "statement"}) || !valid(c.determiner, {"general", "definite", "indefinite"}) || !valid_quantity(c.quantity) || !valid(c.degree, {"very", "more", "less", "equal", "most"}) || (!c.compared_with.empty() && c.degree.empty()) || (!c.predicate_adjective.empty() && c.verb != "copula") || !valid(c.modality, {"ability", "possibility", "permission", "obligation", "recommendation", "intent", "wish"}) || !valid(c.relation, {"location", "destination", "source", "recipient", "instrument", "companion", "topic", "purpose", "possession", "part", "content", "material"}) || !valid(c.question_word, {"person", "thing", "place", "destination", "source", "time", "reason", "manner", "amount", "choice"}) || !valid(c.focus_negation, {"subject", "object", "modality"}) || !valid(c.probability, {"probable", "uncertain"}) || !valid(c.statement_level, {"report", "hypothesis", "fiction", "quote"}) || !valid(c.connective, {"and", "or", "but", "cause", "condition", "concession", "purpose", "temporal"}) || !valid(c.embedding, {"content", "relative", "quote"})) { result.diagnostic = "Nicht unterstützte grammatische Markierung."; return result; }
  if ((!c.connective.empty() || !c.embedding.empty()) && !c.embedded) { result.diagnostic = "Satzverbindung oder Einbettung ohne embedded-Satz."; return result; }
  if (c.embedded && c.connective.empty() && c.embedding.empty()) { result.diagnostic = "embedded-Satz braucht connective oder embedding."; return result; }
  std::vector<std::string> words;
  if (!c.question_word.empty()) words.push_back(form(c.question_word));
  if (!c.statement_level.empty() && c.statement_level != "quote") words.push_back(form(c.statement_level));
  if (!c.probability.empty()) words.push_back(form(c.probability));
  if (!c.emotion.empty()) { words.push_back(form("emotion-marker")); words.push_back(form(c.emotion)); }
  if (!c.social_relation.empty()) { words.push_back(form("social-marker")); words.push_back(form(c.social_relation)); }
  if (!c.goal.empty()) { words.push_back(form("goal-marker")); words.push_back(form(c.goal)); }
  if (c.negation == "statement") words.push_back(form("statement"));
  if (c.speech_act == "request" || c.speech_act == "prohibition") words.push_back(form(c.speech_act));
  if (c.focus_negation == "subject") words.push_back(form("subject-focus"));
  words.push_back(form(c.subject));
  if (c.time == "past" || c.time == "future") words.push_back(form(c.time));
  if (c.aspect != "plain") words.push_back(form(c.aspect));
  if (c.focus_negation == "modality") words.push_back(form("modality-focus"));
  if (!c.modality.empty()) words.push_back(form(c.modality));
  if (c.negation == "predicate") words.push_back(form("predicate"));
  if (c.passive) words.push_back(form("passive"));
  words.push_back(form(c.verb));
  if (c.verb == "copula" && !c.predicate_adjective.empty()) words.push_back(form(c.predicate_adjective));
  if (!c.object.empty()) { if (c.focus_negation == "object") words.push_back(form("object-focus")); if (c.determiner != "general" && !c.determiner.empty()) words.push_back(form(c.determiner)); if (!c.quantity.empty()) append_quantity(words, c.quantity); if (!c.measure.empty()) { words.push_back(form("measure")); words.push_back(form(c.measure)); } words.push_back(form(c.object)); if (!c.adjective.empty()) { if (!c.degree.empty()) words.push_back(form(c.degree)); words.push_back(form(c.adjective)); if (!c.compared_with.empty()) { words.push_back(form("comparison")); words.push_back(form(c.compared_with)); } } }
  if (!c.relation.empty()) { words.push_back(form(c.relation)); if (c.related.empty()) { result.diagnostic = "Unvollständige Relation: related fehlt."; return result; } words.push_back(form(c.related)); }
  if (c.embedded) {
    const auto nested = generate_lunari(*c.embedded); if (!nested.ok) { result.diagnostic = "Eingebetteter Satz: " + nested.diagnostic; return result; }
    std::string nested_text = nested.text; while (!nested_text.empty() && std::ispunct(static_cast<unsigned char>(nested_text.back()))) nested_text.pop_back();
    if (!c.connective.empty()) { words.push_back(form(c.connective == "purpose" ? "purpose-clause" : c.connective)); std::istringstream in(nested_text); for (std::string word; in >> word;) words.push_back(word); }
    else { const std::string marker = c.embedding == "relative" ? "relative" : c.embedding == "quote" ? "quote" : "content"; words.push_back(form(marker + "-open")); std::istringstream in(nested_text); for (std::string word; in >> word;) words.push_back(word); words.push_back(form(marker + "-close")); }
  }
  if (c.speech_act == "question") words.push_back(form("question"));
  std::ostringstream text; for (const auto& word : words) { if (word.empty()) continue; if (text.tellp() > 0) text << ' '; text << word; } text << (c.speech_act == "question" ? '?' : c.speech_act == "request" || c.speech_act == "prohibition" ? '!' : '.');
  result.ok = true; result.text = text.str(); return result;
}

LunariGrammarResult analyse_lunari(const std::string& sentence) {
  LunariGrammarResult result; auto words = split(sentence, ' '); for (auto& word : words) while (!word.empty() && std::ispunct(static_cast<unsigned char>(word.back()))) word.pop_back(); words.erase(std::remove(words.begin(), words.end(), ""), words.end()); if (words.size() == 1 && (key_for(words.front()) == "answer-yes" || key_for(words.front()) == "answer-no")) { result.ok = true; result.clause.speech_act = key_for(words.front()); result.text = "Analysiert als " + std::string(kVersion); return result; } if (words.size() < 2) { result.diagnostic = "Unvollständiger Satz."; return result; }
  LunariClause c;
  const auto join_words = [](const std::vector<std::string>& values) { std::ostringstream out; for (const auto& value : values) { if (out.tellp() > 0) out << ' '; out << value; } return out.str(); };
  const std::array<std::pair<std::string, std::string>, 3> embedded_markers{{{"content-open", "content-close"}, {"relative-open", "relative-close"}, {"quote-open", "quote-close"}}};
  for (const auto& [open, close] : embedded_markers) {
    const auto open_form = form(open), close_form = form(close); auto begin = std::find(words.begin(), words.end(), open_form);
    if (begin == words.end()) continue;
    auto end = begin + 1; int depth = 1;
    for (; end != words.end() && depth > 0; ++end) { if (*end == open_form) ++depth; else if (*end == close_form) --depth; }
    if (depth != 0) { result.diagnostic = "Unvollständige Einbettung: Abschlussmarker " + close_form + " fehlt."; return result; }
    --end;
    std::vector<std::string> inner(begin + 1, end); const auto nested = analyse_lunari(join_words(inner)); if (!nested.ok) { result.diagnostic = "Eingebetteter Satz: " + nested.diagnostic; return result; }
    c.embedding = open == "content-open" ? "content" : open == "relative-open" ? "relative" : "quote"; c.embedded = std::make_shared<LunariClause>(nested.clause); words.erase(begin, end + 1); break;
  }
  if (!c.embedded) {
    for (const auto& connective : {"and", "or", "but", "cause", "condition", "concession", "purpose", "temporal"}) {
      const auto separator = std::find(words.begin(), words.end(), form(std::string(connective) == "purpose" ? "purpose-clause" : connective)); if (separator == words.end()) continue;
      std::vector<std::string> inner(separator + 1, words.end()); const auto nested = analyse_lunari(join_words(inner)); if (!nested.ok) { result.diagnostic = "Verbundener Satz: " + nested.diagnostic; return result; }
      c.connective = connective; c.embedded = std::make_shared<LunariClause>(nested.clause); words.erase(separator, words.end()); break;
    }
  }
  LunariClause parsed; std::size_t i{}; const auto read = [&]() { return i < words.size() ? key_for(words[i]) : std::string{}; };
  if (read() == "person" || read() == "thing" || read() == "place" || read() == "destination" || read() == "source" || read() == "time" || read() == "reason" || read() == "manner" || read() == "amount" || read() == "choice") parsed.question_word = read(), ++i;
  if (read() == "report" || read() == "hypothesis" || read() == "fiction") parsed.statement_level = read(), ++i;
  if (read() == "probable" || read() == "uncertain") parsed.probability = read(), ++i;
  const auto read_scoped_content = [&](const char* marker, std::string& destination, const char* label) -> bool {
    if (read() != marker) return true;
    if (++i >= words.size()) { result.diagnostic = std::string(label) + "-Marker ohne Inhalt."; return false; }
    const auto assessment = assess_neu_lunari_word(words[i]);
    if (!assessment.valid) { result.diagnostic = "Ungültiger " + std::string(label) + "-Inhalt: " + assessment.diagnostic; return false; }
    destination = words[i++]; return true;
  };
  if (!read_scoped_content("emotion-marker", parsed.emotion, "Emotions") || !read_scoped_content("social-marker", parsed.social_relation, "Sozial") || !read_scoped_content("goal-marker", parsed.goal, "Ziel")) return result;
  if (read() == "statement") { parsed.negation = "statement"; ++i; } if (read() == "request" || read() == "prohibition") parsed.speech_act = read(), ++i;
  if (read() == "subject-focus") { parsed.focus_negation = "subject"; ++i; }
  if (i >= words.size()) { result.diagnostic = "Subjekt fehlt."; return result; }
  parsed.subject = key_for(words[i]);
  if (parsed.subject.empty()) {
    const auto historical = kHistoricalAliases.find(words[i]);
    if (historical != kHistoricalAliases.end()) { result.diagnostic = "Historische Form " + words[i] + "; Neu-Lunari-v1-Vorschlag: " + historical->second; return result; }
    const auto assessment = assess_neu_lunari_word(words[i]);
    if (!assessment.valid) { result.diagnostic = "Ungültige Wortform an Subjektposition: " + assessment.diagnostic; return result; }
    parsed.subject = words[i];
  }
  ++i;
  // These markers precede the verb in Neu-Lunari.  Consume them here as a
  // copula can follow them directly (for example: tiri nus gul farim).
  while (i < words.size()) {
    const auto marker = read();
    if (marker == "past" || marker == "future") parsed.time = marker, ++i;
    else if (marker == "progressive" || marker == "completed" || marker == "habitual") parsed.aspect = marker, ++i;
    else if (marker == "modality-focus") parsed.focus_negation = "modality", ++i;
    else if (marker == "ability" || marker == "possibility" || marker == "permission" || marker == "obligation" || marker == "recommendation" || marker == "intent" || marker == "wish") parsed.modality = marker, ++i;
    else if (marker == "predicate") parsed.negation = "predicate", ++i;
    else if (marker == "passive") parsed.passive = true, ++i;
    else break;
  }
  if (parsed.subject == "text-subject" && i < words.size() && key_for(words[i]) == "text-mode") {
    parsed.verb = "text-mode"; ++i;
    for (; i < words.size(); ++i) {
      if (key_for(words[i]) == "question" && i + 1 == words.size()) { parsed.speech_act = "question"; continue; }
      const auto assessment = assess_neu_lunari_word(words[i]);
      if (!assessment.valid) { result.diagnostic = "Ungültiges Textmodus-Wort: " + assessment.diagnostic; return result; }
      if (!parsed.object.empty()) parsed.object += ' ';
      parsed.object += words[i];
    }
    if (parsed.object.empty()) { result.diagnostic = "Textmodus ohne Inhalt."; return result; }
    parsed.embedding = c.embedding; parsed.connective = c.connective; parsed.embedded = std::move(c.embedded);
    result.ok = true; result.clause = std::move(parsed); result.text = "Analysiert als " + std::string(kVersion); return result;
  }
  if (i < words.size() && key_for(words[i]) == "copula") {
    if (i + 1 >= words.size()) { result.diagnostic = "Kopula ohne prädikative Eigenschaft."; return result; }
    const auto assessment = assess_neu_lunari_word(words[i + 1]);
    if (!assessment.valid) { result.diagnostic = "Ungültige prädikative Eigenschaft: " + assessment.diagnostic; return result; }
    parsed.verb = "copula"; parsed.predicate_adjective = words[i + 1];
    words.erase(words.begin() + static_cast<std::ptrdiff_t>(i), words.begin() + static_cast<std::ptrdiff_t>(i + 2));
  }
  // Comparison modifiers are trailing nominal material.  Consume them before
  // the regular parser so an adjective never acquires an invented predicate role.
  for (std::size_t p = i; p + 1 < words.size(); ++p) {
    const auto degree = key_for(words[p]);
    if (degree != "very" && degree != "more" && degree != "less" && degree != "equal" && degree != "most") continue;
    const auto assessment = assess_neu_lunari_word(words[p + 1]);
    if (!assessment.valid) { result.diagnostic = "Ungültige Eigenschaft: " + assessment.diagnostic; return result; }
    parsed.degree = degree; parsed.adjective = words[p + 1]; words.erase(words.begin() + static_cast<std::ptrdiff_t>(p), words.begin() + static_cast<std::ptrdiff_t>(p + 2)); break;
  }
  for (std::size_t p = i; p < words.size(); ++p) {
    if (key_for(words[p]) != "comparison") continue;
    if (parsed.adjective.empty() || p + 1 >= words.size()) { result.diagnostic = "Vergleich ohne Eigenschaft oder Maßstab."; return result; }
    const auto assessment = assess_neu_lunari_word(words[p + 1]);
    if (!assessment.valid) { result.diagnostic = "Ungültiger Vergleichsmaßstab: " + assessment.diagnostic; return result; }
    parsed.compared_with = words[p + 1]; words.erase(words.begin() + static_cast<std::ptrdiff_t>(p), words.begin() + static_cast<std::ptrdiff_t>(p + 2)); break;
  }
  for (; i < words.size(); ++i) { const auto key = read(); if (key == "past" || key == "future") parsed.time = key; else if (key == "progressive" || key == "completed" || key == "habitual") parsed.aspect = key; else if (key == "modality-focus") parsed.focus_negation = "modality"; else if (key == "ability" || key == "possibility" || key == "permission" || key == "obligation" || key == "recommendation" || key == "intent" || key == "wish") parsed.modality = key; else if (key == "predicate") parsed.negation = "predicate"; else if (key == "passive") parsed.passive = true; else if (key == "question") parsed.speech_act = "question"; else if (key == "definite" || key == "indefinite") parsed.determiner = key; else if (key == "plural") parsed.quantity = key; else if (key == "about" || key == "atleast" || key == "atmost" || key == "fraction" || key == "ordinal") { if (++i >= words.size() || !valid_quantity(key + ":" + words[i])) { result.diagnostic = "Ungültige oder unvollständige Mengenangabe."; return result; } parsed.quantity = key + ":" + words[i]; } else if (key == "measure") { if (++i >= words.size()) { result.diagnostic = "Maßmarker ohne Einheit."; return result; } parsed.measure = words[i]; } else if (digits(words[i]) && parsed.object.empty()) parsed.quantity = "exact:" + words[i]; else if (key == "object-focus") parsed.focus_negation = "object"; else if (key == "location" || key == "destination" || key == "source" || key == "recipient" || key == "instrument" || key == "companion" || key == "topic" || key == "purpose" || key == "possession" || key == "part" || key == "content" || key == "material") { parsed.relation = key; if (++i >= words.size()) { result.diagnostic = "Relation ohne Bezug."; return result; } parsed.related = key_for(words[i]); if (parsed.related.empty()) { const auto assessment = assess_neu_lunari_word(words[i]); if (!assessment.valid) { result.diagnostic = "Ungültige Wortform in Relation: " + assessment.diagnostic; return result; } parsed.related = words[i]; } } else if (parsed.verb.empty() || parsed.object.empty()) { const auto assessment = assess_neu_lunari_word(words[i]); if (!assessment.valid) { result.diagnostic = "Ungültige Wortform: " + assessment.diagnostic; return result; } if (parsed.verb.empty()) parsed.verb = words[i]; else parsed.object = words[i]; } else { const auto assessment = assess_neu_lunari_word(words[i]); if (!assessment.valid) { result.diagnostic = "Ungültige Wortform: " + assessment.diagnostic; return result; } parsed.object += ' '; parsed.object += words[i]; } }
  if (parsed.verb.empty()) { result.diagnostic = "Verb fehlt."; return result; } parsed.embedding = c.embedding; parsed.connective = c.connective; parsed.embedded = std::move(c.embedded); result.ok = true; result.clause = std::move(parsed); result.text = "Analysiert als " + std::string(kVersion); return result;
}

LunariWordAssessment assess_neu_lunari_word(const std::string& raw) {
  LunariWordAssessment result; std::string word;
  for (const unsigned char character : raw) { if (!std::isalpha(character)) { result.diagnostic = "nur Kleinbuchstaben a-z erlaubt"; return result; } word.push_back(static_cast<char>(std::tolower(character))); }
  if (word.empty()) { result.diagnostic = "leere Wortform"; return result; }
  std::vector<std::string> syllables; std::size_t i{};
  while (i < word.size()) { const std::size_t start = i; if (onset(word[i])) ++i; if (i >= word.size() || !vowel(word[i])) { result.diagnostic = "jede Silbe braucht einen Vokalkern"; return result; } ++i; if (i < word.size() && coda(word[i]) && (i + 1 == word.size() || (onset(word[i + 1]) && join(word[i], word[i + 1])))) ++i; syllables.push_back(word.substr(start, i - start)); }
  std::ostringstream out; for (std::size_t index = 0; index < syllables.size(); ++index) { if (index) out << '-'; out << syllables[index]; }
  result.valid = true; result.syllables = out.str(); return result;
}

LunariDerivationResult derive_neu_lunari_word(const std::string& root, const std::string& kind, const std::string& qualifier) {
  LunariDerivationResult result;
  const auto root_assessment = assess_neu_lunari_word(root);
  if (!root_assessment.valid) { result.diagnostic = "Ungültiger Grundbegriff: " + root_assessment.diagnostic; return result; }
  std::string candidate;
  if (kind == "agent") candidate = root + "ar";
  else if (kind == "tool") candidate = root + "em";
  else if (kind == "abstract") candidate = root + "im";
  else if (kind == "field") candidate = root + "ul";
  else if (kind == "compound") {
    const auto qualifier_assessment = assess_neu_lunari_word(qualifier);
    if (!qualifier_assessment.valid) { result.diagnostic = "Ungültige nähere Bestimmung: " + qualifier_assessment.diagnostic; return result; }
    candidate = root + (vowel(root.back()) && vowel(qualifier.front()) ? "n" : "") + qualifier;
  } else { result.diagnostic = "Unbekannte Wortbildungsart: " + kind; return result; }
  const auto assessment = assess_neu_lunari_word(candidate);
  if (!assessment.valid) { result.diagnostic = "Wortbildungsgrenze verletzt Lautregel: " + assessment.diagnostic; return result; }
  result.ok = true; result.form = candidate; return result;
}

std::string lunari_clause_trace(const LunariClause& c) {
  std::ostringstream out;
  out << "Neu-Lunari v1 — vorgeschlagen\n";
  out << "Sprechakt: " << c.speech_act << "\nSubjekt: " << c.subject << "\nVerb: " << c.verb;
  if (!c.object.empty()) out << "\nObjekt: " << c.object;
  if (!c.determiner.empty()) out << "\nBestimmtheit: " << c.determiner;
  if (!c.quantity.empty()) out << "\nMenge: " << c.quantity;
  if (!c.measure.empty()) out << "\nMaß: " << c.measure;
  if (!c.adjective.empty()) out << "\nEigenschaft: " << c.adjective;
  if (!c.predicate_adjective.empty()) out << "\nPrädikative Eigenschaft: " << c.predicate_adjective;
  if (!c.degree.empty()) out << "\nGrad: " << c.degree;
  if (!c.compared_with.empty()) out << "\nVergleichsmaßstab: " << c.compared_with;
  if (c.time != "present") out << "\nZeit: " << c.time;
  if (c.aspect != "plain") out << "\nAspekt: " << c.aspect;
  if (!c.modality.empty()) out << "\nModalität: " << c.modality;
  if (!c.probability.empty()) out << "\nWahrscheinlichkeit: " << c.probability;
  if (!c.emotion.empty()) out << "\nEmotion: " << c.emotion;
  if (!c.social_relation.empty()) out << "\nSoziale Beziehung: " << c.social_relation;
  if (!c.goal.empty()) out << "\nZiel: " << c.goal;
  if (c.negation != "none") out << "\nNegation: " << c.negation;
  if (!c.focus_negation.empty()) out << "\nNegationsfokus: " << c.focus_negation;
  if (!c.relation.empty()) out << "\nRelation: " << c.relation << " → " << c.related;
  if (!c.connective.empty()) out << "\nSatzverbindung: " << c.connective;
  if (!c.embedding.empty()) out << "\nEinbettung: " << c.embedding;
  if (c.embedded) out << "\n\nEingebetteter Satz:\n" << lunari_clause_trace(*c.embedded);
  out << "\nStatus: strukturierte Analyse; unbekannte Inhaltslexeme bleiben unbestätigt.";
  return out.str();
}

std::string lunari_grammar_reference() { return "Neu-Lunari v1 (vorgeschlagen)\nSVO; unveränderliche Wörter; Zeit/Aspekt/Modalität/Negation vor Verb; Fragepartikel ke am Satzende.\nFunktionsformen sind Vorschläge, keine rückwirkende Änderung bestätigter Alt-Lunari-Wörter.\nBeispiel: subject=I;verb=see;object=sun;time=past -> mer ren see sun."; }
std::vector<std::string> neu_lunari_reserved_forms() { std::vector<std::string> forms; forms.reserve(kForms.size()); for (const auto& [_, form] : kForms) forms.push_back(form); return forms; }
} // namespace leng
