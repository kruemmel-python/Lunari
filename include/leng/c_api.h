#pragma once

/* Stable, allocation-owning C boundary for local research orchestration. */
#ifdef _WIN32
#  ifdef LENG_API_BUILD
#    define LENG_API __declspec(dllexport)
#  else
#    define LENG_API __declspec(dllimport)
#  endif
#else
#  define LENG_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct leng_engine leng_engine;

/* Returned strings remain valid until the next call on the same engine. */
LENG_API leng_engine* leng_engine_create(unsigned long long seed, int population,
                                         int generations, int training_rounds,
                                         int evaluation_rounds);
LENG_API void leng_engine_destroy(leng_engine* engine);
LENG_API int leng_engine_run(leng_engine* engine);
LENG_API int leng_engine_load(leng_engine* engine, const char* path);
LENG_API int leng_engine_save(const leng_engine* engine, const char* path);
LENG_API int leng_engine_export_results(const leng_engine* engine, const char* directory);
LENG_API const char* leng_engine_report(leng_engine* engine);
/* Neu-Lunari v1 proposal helpers.  Input/output stay owned by engine. */
LENG_API const char* leng_lunari_generate(leng_engine* engine, const char* specification);
LENG_API const char* leng_lunari_analyse(leng_engine* engine, const char* sentence);
LENG_API const char* leng_lunari_assess_word(leng_engine* engine, const char* word);
LENG_API const char* leng_lunari_grammar_reference(leng_engine* engine);
LENG_API const char* leng_engine_last_error(const leng_engine* engine);

#ifdef __cplusplus
}
#endif
