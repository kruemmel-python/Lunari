#include "leng/engine.hpp"

#include <filesystem>
#include <iostream>
#include <string>

int main() {
  try {
    leng::RunConfig config;
    config.population = 4;
    config.generations = 1;
    config.training_rounds = 24;
    config.evaluation_rounds = 12;
    leng::ResearchEngine engine(config);
    engine.run();
    if (engine.generation() != 1 || engine.communication_trace().find("Task success: yes") == std::string::npos) {
      std::cerr << "Android native communication smoke test failed\n";
      return 1;
    }
    const std::filesystem::path state{"/data/local/tmp/leng-device-smoke.leng"};
    engine.save(state);
    leng::ResearchEngine restored(config);
    restored.load(state);
    if (restored.export_language() != engine.export_language()) {
      std::cerr << "Android native persistence smoke test failed\n";
      return 1;
    }
    std::filesystem::remove(state);
    std::cout << "ANDROID_NATIVE_SMOKE_PASSED\n";
    return 0;
  } catch (const std::exception& exception) {
    std::cerr << "ANDROID_NATIVE_SMOKE_FAILED: " << exception.what() << '\n';
    return 1;
  }
}
