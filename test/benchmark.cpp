#define JM_CIRCULAR_BUFFER_CXX14
#include <circular_buffer.hpp>
#include <benchmark/benchmark.h>
#include <ctime>

namespace {
  constexpr size_t k1kB = 1000;
  constexpr size_t k1MB = k1kB * 1000;
  constexpr size_t k1GB = k1MB * 1000;
  constexpr size_t k10GB = k1GB * 10;
  std::string generateRandomString() {
    return "Hello World number " + std::to_string(rand());
  }
void BM_StaticCircleBufferCreation_k1kB(benchmark::State& state) {
  for (auto _ : state)
    jm::StaticCircleBuffer<std::string, k1kB> data;
}

void BM_DynamicCircleBufferCreation_k1kB(benchmark::State& state) {
  for (auto _ : state)
    jm::DynamicCircleBuffer<std::string, k1kB> data;
}

void BM_DynamicCircleBufferCreation_k1MB(benchmark::State& state) {
  for (auto _ : state)
    jm::DynamicCircleBuffer<std::string, k1MB> data;
}

void BM_DynamicCircleBufferCreation_k1GB(benchmark::State& state) {
  for (auto _ : state) 
    jm::DynamicCircleBuffer<std::string, k1GB> data;
}

void BM_StaticCircleBufferCreation_k1kB_push_back(benchmark::State& state) {
  jm::StaticCircleBuffer<std::string, k1kB> data;
  srand(time(0));
  for (auto _ : state) {
    for (size_t i = 0; i < state.range(0); i++) {
      data.push_back(generateRandomString());
    }
  }
}

void BM_DynamicCircleBufferCreation_k1kB_push_back(benchmark::State& state) {
  jm::DynamicCircleBuffer<std::string, k1kB> data;
  srand(time(0));
  for (auto _ : state) {
    for (size_t i = 0; i < state.range(0); i++) {
      data.push_back(generateRandomString());
    }
  }
}

}
// Register the function as a benchmark
BENCHMARK(BM_StaticCircleBufferCreation_k1kB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1MB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1GB);

BENCHMARK(BM_StaticCircleBufferCreation_k1kB_push_back)->RangeMultiplier(2)->Range(8, 8<<10);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB_push_back)->RangeMultiplier(2)->Range(8, 8<<10);


 
BENCHMARK_MAIN();