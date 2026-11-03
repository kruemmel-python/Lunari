#include "leng/c_api.h"
#include "leng/engine.hpp"
#include "leng/lunari_grammar.hpp"

#include <exception>
#include <memory>
#include <string>

struct leng_engine {
  std::unique_ptr<leng::ResearchEngine> value;
  std::string scratch;
  std::string error;
};

namespace {
template <typename Action>
int invoke(leng_engine* engine, Action&& action) {
  if (!engine || !engine->value) return 0;
  try { action(); engine->error.clear(); return 1; }
  catch (const std::exception& exception) { engine->error = exception.what(); return 0; }
  catch (...) { engine->error = "unknown native engine error"; return 0; }
}
}

leng_engine* leng_engine_create(unsigned long long seed, int population, int generations,
                                int training_rounds, int evaluation_rounds) {
  try {
    leng::RunConfig config;
    config.seed = seed;
    config.population = population;
    config.generations = generations;
    config.training_rounds = training_rounds;
    config.evaluation_rounds = evaluation_rounds;
    auto engine = std::make_unique<leng_engine>();
    engine->value = std::make_unique<leng::ResearchEngine>(config);
    return engine.release();
  } catch (...) { return nullptr; }
}

void leng_engine_destroy(leng_engine* engine) { delete engine; }
int leng_engine_run(leng_engine* engine) { return invoke(engine, [&] { engine->value->run(); }); }
int leng_engine_load(leng_engine* engine, const char* path) {
  return path ? invoke(engine, [&] { engine->value->load(path); }) : 0;
}
int leng_engine_save(const leng_engine* engine, const char* path) {
  return (engine && path) ? invoke(const_cast<leng_engine*>(engine), [&] { engine->value->save(path); }) : 0;
}
int leng_engine_export_results(const leng_engine* engine, const char* directory) {
  return (engine && directory) ? invoke(const_cast<leng_engine*>(engine), [&] { engine->value->export_results(directory); }) : 0;
}
const char* leng_engine_report(leng_engine* engine) {
  if (!engine || !engine->value) return "";
  if (!invoke(engine, [&] { engine->scratch = engine->value->report(); })) return "";
  return engine->scratch.c_str();
}
const char* leng_lunari_generate(leng_engine* engine, const char* specification) {
  if (!engine || !specification) return "";
  if (!invoke(engine, [&] { const auto parsed = leng::parse_lunari_spec(specification); if (!parsed.ok) throw std::runtime_error(parsed.diagnostic); const auto generated = leng::generate_lunari(parsed.clause); if (!generated.ok) throw std::runtime_error(generated.diagnostic); engine->scratch = generated.text; })) return "";
  return engine->scratch.c_str();
}
const char* leng_lunari_analyse(leng_engine* engine, const char* sentence) {
  if (!engine || !sentence) return "";
  if (!invoke(engine, [&] { const auto result = leng::analyse_lunari(sentence); if (!result.ok) throw std::runtime_error(result.diagnostic); engine->scratch = result.text; })) return "";
  return engine->scratch.c_str();
}
const char* leng_lunari_assess_word(leng_engine* engine, const char* word) {
  if (!engine || !word) return "";
  if (!invoke(engine, [&] { const auto result = leng::assess_neu_lunari_word(word); if (!result.valid) throw std::runtime_error(result.diagnostic); engine->scratch = result.syllables; })) return "";
  return engine->scratch.c_str();
}
const char* leng_lunari_grammar_reference(leng_engine* engine) {
  if (!engine) return "";
  if (!invoke(engine, [&] { engine->scratch = leng::lunari_grammar_reference(); })) return "";
  return engine->scratch.c_str();
}
const char* leng_engine_last_error(const leng_engine* engine) {
  return engine ? engine->error.c_str() : "invalid engine handle";
}
