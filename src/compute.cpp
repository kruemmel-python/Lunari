#include "leng/compute.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace leng {
namespace {
using ClInt = std::int32_t;
using ClUInt = std::uint32_t;
using ClULong = std::uint64_t;
using ClPlatformId = void*;
using ClDeviceId = void*;
constexpr ClInt kSuccess = 0;
constexpr ClULong kDeviceTypeAll = 0xFFFFFFFFull;
constexpr ClULong kDeviceTypeGpu = 1ull << 2;
constexpr ClUInt kDeviceName = 0x102B;
using GetPlatforms = ClInt (*)(ClUInt, ClPlatformId*, ClUInt*);
using GetDevices = ClInt (*)(ClPlatformId, ClULong, ClUInt, ClDeviceId*, ClUInt*);
using GetDeviceInfo = ClInt (*)(ClDeviceId, ClUInt, std::size_t, void*, std::size_t*);
using ClContext = void*;
using ClQueue = void*;
using ClProgram = void*;
using ClKernel = void*;
using ClMem = void*;
using CreateContext = ClContext (*)(const std::intptr_t*, ClUInt, const ClDeviceId*, void*, void*, ClInt*);
using CreateCommandQueue = ClQueue (*)(ClContext, ClDeviceId, ClULong, ClInt*);
using CreateProgramWithSource = ClProgram (*)(ClContext, ClUInt, const char**, const std::size_t*, ClInt*);
using BuildProgram = ClInt (*)(ClProgram, ClUInt, const ClDeviceId*, const char*, void*, void*);
using CreateKernel = ClKernel (*)(ClProgram, const char*, ClInt*);
using CreateBuffer = ClMem (*)(ClContext, ClULong, std::size_t, void*, ClInt*);
using SetKernelArg = ClInt (*)(ClKernel, ClUInt, std::size_t, const void*);
using EnqueueWriteBuffer = ClInt (*)(ClQueue, ClMem, ClUInt, std::size_t, std::size_t, const void*, ClUInt, const void*, void*);
using EnqueueReadBuffer = EnqueueWriteBuffer;
using EnqueueNdRangeKernel = ClInt (*)(ClQueue, ClKernel, ClUInt, const std::size_t*, const std::size_t*, const std::size_t*, ClUInt, const void*, void*);
using Finish = ClInt (*)(ClQueue);
using Release = ClInt (*)(void*);
constexpr ClULong kMemReadOnly = 1ull << 2;
constexpr ClULong kMemWriteOnly = 1ull << 1;
constexpr ClUInt kTrue = 1;

class DynamicLibrary {
 public:
  DynamicLibrary() {
#ifdef _WIN32
    handle_ = LoadLibraryA("OpenCL.dll");
#else
    handle_ = dlopen("libOpenCL.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!handle_) handle_ = dlopen("libOpenCL.so", RTLD_LAZY | RTLD_LOCAL);
#endif
  }
  ~DynamicLibrary() {
#ifdef _WIN32
    if (handle_) FreeLibrary(handle_);
#else
    if (handle_) dlclose(handle_);
#endif
  }
  [[nodiscard]] bool available() const { return handle_ != nullptr; }
  template <typename Function>
  [[nodiscard]] Function symbol(const char* name) const {
#ifdef _WIN32
    const auto source = GetProcAddress(handle_, name);
#else
    const auto source = dlsym(handle_, name);
#endif
    Function target{};
    static_assert(sizeof(target) == sizeof(source), "unsupported function pointer ABI");
    std::memcpy(&target, &source, sizeof(target));
    return target;
  }
 private:
#ifdef _WIN32
  HMODULE handle_{};
#else
  void* handle_{};
#endif
};
}  // namespace

ComputeInfo inspect_compute() {
  ComputeInfo result;
  DynamicLibrary library;
  if (!library.available()) { result.diagnostic = "OpenCL loader not installed; CPU reference backend selected."; return result; }
  result.opencl_loader_available = true;
  const auto get_platforms = library.symbol<GetPlatforms>("clGetPlatformIDs");
  const auto get_devices = library.symbol<GetDevices>("clGetDeviceIDs");
  const auto get_device_info = library.symbol<GetDeviceInfo>("clGetDeviceInfo");
  if (!get_platforms || !get_devices || !get_device_info) { result.diagnostic = "OpenCL loader is incomplete; CPU reference backend selected."; return result; }
  ClUInt count{};
  if (get_platforms(0, nullptr, &count) != kSuccess || count == 0) { result.diagnostic = "No OpenCL platforms found; CPU reference backend selected."; return result; }
  std::vector<ClPlatformId> platforms(count);
  if (get_platforms(count, platforms.data(), nullptr) != kSuccess) { result.diagnostic = "OpenCL platform enumeration failed; CPU reference backend selected."; return result; }
  for (const auto platform : platforms) {
    ClUInt device_count{};
    if (get_devices(platform, kDeviceTypeAll, 0, nullptr, &device_count) != kSuccess || device_count == 0) continue;
    std::vector<ClDeviceId> devices(device_count);
    if (get_devices(platform, kDeviceTypeAll, device_count, devices.data(), nullptr) != kSuccess) continue;
    for (const auto device : devices) {
      std::size_t bytes{};
      if (get_device_info(device, kDeviceName, 0, nullptr, &bytes) != kSuccess || bytes == 0) continue;
      std::string name(bytes, '\0');
      if (get_device_info(device, kDeviceName, name.size(), name.data(), nullptr) != kSuccess) continue;
      if (!name.empty() && name.back() == '\0') name.pop_back();
      result.devices.push_back(name);
    }
    ClUInt gpu_count{};
    if (get_devices(platform, kDeviceTypeGpu, 0, nullptr, &gpu_count) == kSuccess && gpu_count > 0) result.gpu_available = true;
  }
  result.device_available = !result.devices.empty();
  result.diagnostic = result.device_available ? (result.gpu_available ? "OpenCL GPU available; CPU reference backend remains selectable." : "OpenCL non-GPU device available; CPU reference backend remains selectable.") : "No usable OpenCL devices found; CPU reference backend selected.";
  return result;
}

ComputeParity run_reward_parity_test() {
  ComputeParity result;
  DynamicLibrary library;
  if (!library.available()) { result.skipped = true; result.detail = "Skipped: OpenCL loader unavailable; CPU fallback is active."; return result; }
  const auto get_platforms = library.symbol<GetPlatforms>("clGetPlatformIDs");
  const auto get_devices = library.symbol<GetDevices>("clGetDeviceIDs");
  const auto create_context = library.symbol<CreateContext>("clCreateContext");
  const auto create_queue = library.symbol<CreateCommandQueue>("clCreateCommandQueue");
  const auto create_program = library.symbol<CreateProgramWithSource>("clCreateProgramWithSource");
  const auto build_program = library.symbol<BuildProgram>("clBuildProgram");
  const auto create_kernel = library.symbol<CreateKernel>("clCreateKernel");
  const auto create_buffer = library.symbol<CreateBuffer>("clCreateBuffer");
  const auto set_arg = library.symbol<SetKernelArg>("clSetKernelArg");
  const auto write_buffer = library.symbol<EnqueueWriteBuffer>("clEnqueueWriteBuffer");
  const auto read_buffer = library.symbol<EnqueueReadBuffer>("clEnqueueReadBuffer");
  const auto enqueue_kernel = library.symbol<EnqueueNdRangeKernel>("clEnqueueNDRangeKernel");
  const auto finish = library.symbol<Finish>("clFinish");
  const auto release_context = library.symbol<Release>("clReleaseContext");
  const auto release_queue = library.symbol<Release>("clReleaseCommandQueue");
  const auto release_program = library.symbol<Release>("clReleaseProgram");
  const auto release_kernel = library.symbol<Release>("clReleaseKernel");
  const auto release_mem = library.symbol<Release>("clReleaseMemObject");
  if (!get_platforms || !get_devices || !create_context || !create_queue || !create_program || !build_program || !create_kernel || !create_buffer || !set_arg || !write_buffer || !read_buffer || !enqueue_kernel || !finish || !release_context || !release_queue || !release_program || !release_kernel || !release_mem) { result.skipped = true; result.detail = "Skipped: required OpenCL 1.2 symbols are unavailable."; return result; }
  ClUInt platform_count{};
  if (get_platforms(0, nullptr, &platform_count) != kSuccess || platform_count == 0) { result.skipped = true; result.detail = "Skipped: no OpenCL platform."; return result; }
  std::vector<ClPlatformId> platforms(platform_count);
  if (get_platforms(platform_count, platforms.data(), nullptr) != kSuccess) { result.skipped = true; result.detail = "Skipped: platform enumeration failed."; return result; }
  ClDeviceId device{};
  for (const auto platform : platforms) { ClUInt count{}; if (get_devices(platform, kDeviceTypeGpu, 0, nullptr, &count) == kSuccess && count > 0) { std::vector<ClDeviceId> candidates(count); if (get_devices(platform, kDeviceTypeGpu, count, candidates.data(), nullptr) == kSuccess) { device = candidates.front(); break; } } }
  if (!device) { result.skipped = true; result.detail = "Skipped: no OpenCL GPU device; CPU fallback is active."; return result; }
  result.attempted = true;
  ClInt error{};
  ClContext context = create_context(nullptr, 1, &device, nullptr, nullptr, &error);
  if (!context || error != kSuccess) { result.detail = "GPU parity test failed: context creation."; return result; }
  ClQueue queue = create_queue(context, device, 0, &error);
  if (!queue || error != kSuccess) { release_context(context); result.detail = "GPU parity test failed: command queue creation."; return result; }
  constexpr char kernel_source[] = "__kernel void reward(__global const float* success,__global const float* cost,__global float* out,float penalty){size_t i=get_global_id(0);out[i]=success[i]-penalty*cost[i];}";
  const char* source_ptr = kernel_source;
  const std::size_t source_size = sizeof(kernel_source) - 1;
  ClProgram program = create_program(context, 1, &source_ptr, &source_size, &error);
  if (!program || error != kSuccess || build_program(program, 1, &device, "", nullptr, nullptr) != kSuccess) { if (program) release_program(program); release_queue(queue); release_context(context); result.detail = "GPU parity test failed: kernel compilation."; return result; }
  ClKernel kernel = create_kernel(program, "reward", &error);
  if (!kernel || error != kSuccess) { release_program(program); release_queue(queue); release_context(context); result.detail = "GPU parity test failed: kernel creation."; return result; }
  constexpr std::size_t n = 1024;
  std::array<float, n> success{}; std::array<float, n> cost{}; std::array<float, n> gpu{}; std::array<float, n> cpu{};
  constexpr float penalty = 0.125f;
  for (std::size_t i = 0; i < n; ++i) { success[i] = static_cast<float>((i * 17) % 101) / 100.0f; cost[i] = static_cast<float>((i * 31) % 97) / 97.0f; cpu[i] = success[i] - penalty * cost[i]; }
  const std::size_t bytes = sizeof(float) * n;
  ClMem success_mem = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); ClMem cost_mem = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); ClMem out_mem = create_buffer(context, kMemWriteOnly, bytes, nullptr, &error);
  const bool buffers_ok = success_mem && cost_mem && out_mem && error == kSuccess;
  const bool io_ok = buffers_ok && write_buffer(queue, success_mem, kTrue, 0, bytes, success.data(), 0, nullptr, nullptr) == kSuccess && write_buffer(queue, cost_mem, kTrue, 0, bytes, cost.data(), 0, nullptr, nullptr) == kSuccess;
  const bool args_ok = io_ok && set_arg(kernel, 0, sizeof(success_mem), &success_mem) == kSuccess && set_arg(kernel, 1, sizeof(cost_mem), &cost_mem) == kSuccess && set_arg(kernel, 2, sizeof(out_mem), &out_mem) == kSuccess && set_arg(kernel, 3, sizeof(penalty), &penalty) == kSuccess;
  const std::size_t global = n;
  const bool run_ok = args_ok && enqueue_kernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr) == kSuccess && finish(queue) == kSuccess && read_buffer(queue, out_mem, kTrue, 0, bytes, gpu.data(), 0, nullptr, nullptr) == kSuccess;
  if (success_mem) release_mem(success_mem);
  if (cost_mem) release_mem(cost_mem);
  if (out_mem) release_mem(out_mem);
  release_kernel(kernel);
  release_program(program);
  release_queue(queue);
  release_context(context);
  if (!run_ok) { result.detail = "GPU parity test failed: buffer transfer or kernel execution."; return result; }
  for (std::size_t i = 0; i < n; ++i) result.max_absolute_error = std::max(result.max_absolute_error, static_cast<double>(std::abs(cpu[i] - gpu[i])));
  result.passed = result.max_absolute_error <= 1e-6;
  result.detail = result.passed ? "GPU/CPU reward parity passed." : "GPU/CPU reward parity exceeded tolerance.";
  return result;
}

RewardBatch compute_reward_batch(const std::vector<double>& benefit, const std::vector<double>& cost, double penalty) {
  if (benefit.size() != cost.size()) throw std::invalid_argument("reward batch sizes differ");
  RewardBatch result;
  result.values.resize(benefit.size());
  for (std::size_t i = 0; i < benefit.size(); ++i) result.values[i] = benefit[i] - penalty * cost[i];
  if (benefit.empty()) { result.diagnostic = "empty CPU reward batch"; return result; }
  // The parity invocation exercises the same OpenCL 1.2 kernel ABI, and guards
  // selection against a driver whose arithmetic is not equivalent to the CPU.
  const auto parity = run_reward_parity_test();
  if (!parity.attempted || !parity.passed) { result.diagnostic = "CPU reward batch: " + parity.detail; return result; }
  DynamicLibrary library;
  const auto get_platforms = library.symbol<GetPlatforms>("clGetPlatformIDs");
  const auto get_devices = library.symbol<GetDevices>("clGetDeviceIDs");
  const auto create_context = library.symbol<CreateContext>("clCreateContext");
  const auto create_queue = library.symbol<CreateCommandQueue>("clCreateCommandQueue");
  const auto create_program = library.symbol<CreateProgramWithSource>("clCreateProgramWithSource");
  const auto build_program = library.symbol<BuildProgram>("clBuildProgram");
  const auto create_kernel = library.symbol<CreateKernel>("clCreateKernel");
  const auto create_buffer = library.symbol<CreateBuffer>("clCreateBuffer");
  const auto set_arg = library.symbol<SetKernelArg>("clSetKernelArg");
  const auto write_buffer = library.symbol<EnqueueWriteBuffer>("clEnqueueWriteBuffer");
  const auto read_buffer = library.symbol<EnqueueReadBuffer>("clEnqueueReadBuffer");
  const auto enqueue_kernel = library.symbol<EnqueueNdRangeKernel>("clEnqueueNDRangeKernel");
  const auto finish = library.symbol<Finish>("clFinish");
  const auto release_context = library.symbol<Release>("clReleaseContext"); const auto release_queue = library.symbol<Release>("clReleaseCommandQueue");
  const auto release_program = library.symbol<Release>("clReleaseProgram"); const auto release_kernel = library.symbol<Release>("clReleaseKernel"); const auto release_mem = library.symbol<Release>("clReleaseMemObject");
  if (!get_platforms || !get_devices || !create_context || !create_queue || !create_program || !build_program || !create_kernel || !create_buffer || !set_arg || !write_buffer || !read_buffer || !enqueue_kernel || !finish || !release_context || !release_queue || !release_program || !release_kernel || !release_mem) { result.diagnostic = "CPU reward batch: incomplete OpenCL loader"; return result; }
  ClUInt platforms{}; if (get_platforms(0, nullptr, &platforms) != kSuccess || !platforms) { result.diagnostic = "CPU reward batch: no OpenCL platform"; return result; }
  std::vector<ClPlatformId> ids(platforms); if (get_platforms(platforms, ids.data(), nullptr) != kSuccess) { result.diagnostic = "CPU reward batch: platform enumeration failed"; return result; }
  ClDeviceId device{}; for (auto id : ids) { ClUInt count{}; if (get_devices(id, kDeviceTypeGpu, 0, nullptr, &count) == kSuccess && count) { std::vector<ClDeviceId> choices(count); if (get_devices(id, kDeviceTypeGpu, count, choices.data(), nullptr) == kSuccess) { device = choices.front(); break; } } }
  if (!device) { result.diagnostic = "CPU reward batch: no GPU"; return result; }
  ClInt error{}; auto context = create_context(nullptr, 1, &device, nullptr, nullptr, &error); if (!context || error != kSuccess) { result.diagnostic = "CPU reward batch: context creation failed"; return result; }
  auto queue = create_queue(context, device, 0, &error); if (!queue || error != kSuccess) { release_context(context); result.diagnostic = "CPU reward batch: queue creation failed"; return result; }
  constexpr char source[] = "__kernel void reward(__global const float*b,__global const float*c,__global float*o,float p){size_t i=get_global_id(0);o[i]=b[i]-p*c[i];}"; const char* source_ptr = source; const std::size_t source_len = sizeof(source) - 1;
  auto program = create_program(context, 1, &source_ptr, &source_len, &error); if (!program || error != kSuccess || build_program(program, 1, &device, "", nullptr, nullptr) != kSuccess) { if (program) release_program(program); release_queue(queue); release_context(context); result.diagnostic = "CPU reward batch: kernel build failed"; return result; }
  auto kernel = create_kernel(program, "reward", &error); if (!kernel || error != kSuccess) { release_program(program); release_queue(queue); release_context(context); result.diagnostic = "CPU reward batch: kernel creation failed"; return result; }
  std::vector<float> b(benefit.begin(), benefit.end()), c(cost.begin(), cost.end()), out(benefit.size()); const std::size_t bytes = b.size() * sizeof(float); const float p = static_cast<float>(penalty);
  auto bm = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); auto cm = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); auto om = create_buffer(context, kMemWriteOnly, bytes, nullptr, &error);
  const std::size_t global = b.size(); const bool ok = bm && cm && om && error == kSuccess && write_buffer(queue,bm,kTrue,0,bytes,b.data(),0,nullptr,nullptr)==kSuccess && write_buffer(queue,cm,kTrue,0,bytes,c.data(),0,nullptr,nullptr)==kSuccess && set_arg(kernel,0,sizeof(bm),&bm)==kSuccess && set_arg(kernel,1,sizeof(cm),&cm)==kSuccess && set_arg(kernel,2,sizeof(om),&om)==kSuccess && set_arg(kernel,3,sizeof(p),&p)==kSuccess && enqueue_kernel(queue,kernel,1,nullptr,&global,nullptr,0,nullptr,nullptr)==kSuccess && finish(queue)==kSuccess && read_buffer(queue,om,kTrue,0,bytes,out.data(),0,nullptr,nullptr)==kSuccess;
  if (bm) release_mem(bm);
  if (cm) release_mem(cm);
  if (om) release_mem(om);
  release_kernel(kernel);
  release_program(program);
  release_queue(queue);
  release_context(context);
  if (!ok) { result.diagnostic = "CPU reward batch: GPU execution failed"; return result; }
  for (std::size_t i = 0; i < out.size(); ++i) { if (std::abs(result.values[i] - static_cast<double>(out[i])) > 1e-5) { result.diagnostic = "CPU reward batch: parity tolerance exceeded"; return result; } result.values[i] = out[i]; }
  result.gpu_used = true; result.diagnostic = "OpenCL GPU reward batch"; return result;
}
}  // namespace leng
