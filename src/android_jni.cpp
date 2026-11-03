#include "leng/lunari_grammar.hpp"

#include <jni.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_set>

namespace {
std::mutex lexicon_mutex;
std::string loaded_lexicon_path;
std::unordered_set<std::string> learned_this_session;

std::string from_java(JNIEnv* environment, jstring value) {
  if (!value) return {};
  const char* text = environment->GetStringUTFChars(value, nullptr);
  if (!text) return {};
  std::string result(text);
  environment->ReleaseStringUTFChars(value, text);
  return result;
}

jstring to_java(JNIEnv* environment, const std::string& value) { return environment->NewStringUTF(value.c_str()); }

void persist_lexemes(const std::string& text, const std::filesystem::path& path) {
  std::ofstream output(path, std::ios::app);
  if (!output) throw std::runtime_error("Neu-Lunari-Lexikon kann nicht gespeichert werden.");
  std::string word;
  const auto save = [&] {
    if (word.empty()) return;
    const auto form = leng::encode_german_neu_lunari_lexeme(word);
    if (!form.empty() && learned_this_session.insert(word).second) output << word << '\t' << form << '\n';
    word.clear();
  };
  for (const unsigned char character : text) {
    if (std::isspace(character) || std::string_view{".,;:!?()[]{}\"'"}.find(static_cast<char>(character)) != std::string_view::npos) save();
    else word.push_back(static_cast<char>(character));
  }
  save();
  if (!output) throw std::runtime_error("Neu-Lunari-Lexikon konnte nicht vollständig gespeichert werden.");
}
}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_org_leng_research_MainActivity_translateNative(JNIEnv* environment, jobject,
                                                     jstring source, jstring lexicon_file,
                                                     jboolean into_lunari) {
  try {
    const std::string input = from_java(environment, source);
    const std::string path = from_java(environment, lexicon_file);
    if (input.empty()) return to_java(environment, "FEHLER: Bitte Text eingeben.");
    if (path.empty() || !std::filesystem::exists(path)) return to_java(environment, "FEHLER: Das Neu-Lunari-Lexikon ist nicht verfügbar.");
    std::lock_guard lock(lexicon_mutex);
    if (loaded_lexicon_path != path) { leng::load_neu_lunari_lexicon(path); loaded_lexicon_path = path; learned_this_session.clear(); }
    if (into_lunari == JNI_TRUE) {
      const auto result = leng::translate_german_lunari_text(input);
      if (!result.ok) return to_java(environment, "FEHLER: " + result.diagnostic);
      persist_lexemes(input, path);
      return to_java(environment, result.text);
    }
    const auto result = leng::translate_lunari_german_text(input);
    return to_java(environment, result.ok ? result.text : "FEHLER: " + result.diagnostic);
  } catch (const std::exception& error) {
    return to_java(environment, std::string("FEHLER: ") + error.what());
  } catch (...) {
    return to_java(environment, "FEHLER: Unbekannter nativer Fehler.");
  }
}

extern "C" JNIEXPORT void JNICALL
Java_org_leng_research_MainActivity_closeNative(JNIEnv*, jobject) {
  std::lock_guard lock(lexicon_mutex);
  loaded_lexicon_path.clear();
  learned_this_session.clear();
}
