#include "leng/engine.hpp"

#include <jni.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

namespace {
std::mutex engine_mutex;
std::unique_ptr<leng::ResearchEngine> engine;
std::string loaded_state_path;

std::string from_java(JNIEnv* environment, jstring value) {
  if (!value) return {};
  const char* text = environment->GetStringUTFChars(value, nullptr);
  if (!text) return {};
  std::string result(text);
  environment->ReleaseStringUTFChars(value, text);
  return result;
}

jstring to_java(JNIEnv* environment, const std::string& value) {
  return environment->NewStringUTF(value.c_str());
}

std::string extract_block(const std::string& report, const std::string& header,
                          const std::string& footer) {
  const std::size_t start = report.find(header);
  if (start == std::string::npos) return report;
  const std::size_t content_start = start + header.size();
  const std::size_t end = report.find(footer, content_start);
  return report.substr(content_start, end == std::string::npos ? std::string::npos : end - content_start);
}
}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_org_leng_research_MainActivity_translateNative(JNIEnv* environment, jobject,
                                                     jstring source, jstring state_file,
                                                     jboolean into_lunari) {
  try {
    const std::string input = from_java(environment, source);
    const std::string path = from_java(environment, state_file);
    if (input.empty()) return to_java(environment, "FEHLER: Bitte Text eingeben.");
    if (path.empty() || !std::filesystem::exists(path)) {
      return to_java(environment, "FEHLER: Der Lunari-Datenstand ist nicht verfügbar.");
    }
    std::lock_guard lock(engine_mutex);
    if (!engine || loaded_state_path != path) {
      engine = std::make_unique<leng::ResearchEngine>();
      engine->load(path);
      loaded_state_path = path;
    }
    if (into_lunari == JNI_TRUE) {
      const std::string report = engine->translate_text(input, "constructed");
      if (report.find("newly learned tokens=0") == std::string::npos) engine->save(path);
      return to_java(environment, extract_block(report, "Lunari text:\n", "\n-- end Lunari text --"));
    }
    const std::string report = engine->translate_text(input, "de");
    return to_java(environment, extract_block(report, "Deutsch text:\n", "\n-- end translated text --"));
  } catch (const std::exception& error) {
    return to_java(environment, std::string("FEHLER: ") + error.what());
  } catch (...) {
    return to_java(environment, "FEHLER: Unbekannter nativer Fehler.");
  }
}

extern "C" JNIEXPORT void JNICALL
Java_org_leng_research_MainActivity_closeNative(JNIEnv*, jobject) {
  std::lock_guard lock(engine_mutex);
  engine.reset();
  loaded_state_path.clear();
}
