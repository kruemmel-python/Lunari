#pragma once

#include <string>
#include <vector>

namespace leng {

struct ComputeInfo {
  bool opencl_loader_available{};
  bool device_available{};
  bool gpu_available{};
  std::vector<std::string> devices;
  std::string diagnostic;
};

struct ComputeParity {
  bool attempted{};
  bool passed{};
  bool skipped{};
  double max_absolute_error{};
  std::string detail;
};

struct RewardBatch {
  std::vector<double> values;
  bool gpu_used{};
  std::string diagnostic;
};

// Runtime probe only: it does not select a hard-coded device index and does
// not make the application depend on an OpenCL ICD at startup.
[[nodiscard]] ComputeInfo inspect_compute();
// Executes the same reward-vector calculation on a dynamically selected GPU
// and on CPU, then compares each result with a defined tolerance.
[[nodiscard]] ComputeParity run_reward_parity_test();
// Evaluates a whole candidate-fitness batch on an automatically selected GPU.
// It returns the numerically equivalent CPU result if OpenCL is unavailable.
[[nodiscard]] RewardBatch compute_reward_batch(const std::vector<double>& benefit,
                                               const std::vector<double>& cost,
                                               double penalty);

}  // namespace leng
