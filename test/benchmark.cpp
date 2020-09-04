#define JM_CIRCULAR_BUFFER_CXX14
#include <circular_buffer.hpp>
#include <benchmark/benchmark.h>

#include <iostream>
#include <exception>

#include <ctime>

namespace {
  constexpr size_t k1kB = 1000;
  constexpr size_t k1MB = k1kB * 1000;
  constexpr size_t k1GB = k1MB * 1000;
  constexpr size_t k10GB = k1GB * 10;
  char generateRandomString() {
    return rand() % 255;
  }
  void BM_StaticCircleBufferCreation_k1kB(benchmark::State& state) {
    for (auto _ : state)
      jm::static_circular_buffer<char, k1kB> data;
  }

  void BM_DynamicCircleBufferCreation_k1kB(benchmark::State& state) {
    for (auto _ : state) {
      try {
        jm::dynamic_circular_buffer<char> data(k1kB);
      }
      catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
    }
  }

  void BM_DynamicCircleBufferCreation_k1MB(benchmark::State& state) {
    for (auto _ : state) {
      try {
        jm::dynamic_circular_buffer<char> data;
        data.reserve(k1MB);
      }
      catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
    }
  }

  void BM_StaticCircleBufferCreation_k1kB_push_back(benchmark::State& state) {
    srand(time(0));
    for (auto _ : state) {
      jm::static_circular_buffer<char, k1kB> data;
      for (size_t i = 0; i < state.range(0); i++) {
        data.push_back(generateRandomString());
      }
    }
  }

  void BM_DynamicCircleBufferCreation_k1kB_push_back(benchmark::State& state) {

    try {
      srand(time(0));
      for (auto _ : state) {
        jm::dynamic_circular_buffer<char> data;
        data.reserve(k1kB);
        for (size_t i = 0; i < state.range(0); i++) {
          data.push_back(generateRandomString());
        }
      }
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void BM_StaticCircleBufferCreation_k1kB_iteration(benchmark::State& state) {

    try {
      for (auto _ : state) {
        jm::static_circular_buffer<char, k1kB> data;
        state.PauseTiming();
        for (size_t i = 0; i < state.range(0); i++) {
          data.push_back(generateRandomString());
        }
        state.ResumeTiming();

        std::for_each(data.begin(), data.end(),[](auto& value){
          value = generateRandomString();
        });
      }
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void BM_DynamicCircleBufferCreation_k1kB_iteration(benchmark::State& state) {

    try {
      for (auto _ : state) {
        jm::dynamic_circular_buffer<char> data(k1kB);
        state.PauseTiming();
        for (size_t i = 0; i < state.range(0); i++) {
          data.push_back(generateRandomString());
        }
        state.ResumeTiming();

        std::for_each(data.begin(), data.end(), [](auto& value) {
          value = generateRandomString();
        });
      }
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

}

#include <Eigen/Geometry>
#include <Eigen/StdVector>

void BM_STDVectorEigen_1K_elements_without_Allocator(benchmark::State& state) {

  try {
    for (auto _ : state) {
      state.PauseTiming();
      const auto randomValue = Eigen::Vector4f::Random();
      std::vector<Eigen::Vector4f> data(state.range(0), randomValue);
      state.ResumeTiming();

      std::for_each(data.begin(), data.end(), [&](auto& value) {
        const auto randomValue = Eigen::Vector4f::Random();
        benchmark::DoNotOptimize(value.dot(randomValue));
      });
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void BM_STDVectorEigen_1K_elements_with_Allocator(benchmark::State& state) {

  try {
    for (auto _ : state) {
      state.PauseTiming();
      const auto randomValue = Eigen::Vector4f::Random();
      std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> data(state.range(0), randomValue);
      state.ResumeTiming();

      std::for_each(data.begin(), data.end(), [](auto& value) {
        const auto randomValue = Eigen::Vector4f::Random();
        benchmark::DoNotOptimize(value.dot(randomValue));
      });
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void BM_DynamicCircleBufferEigen_1K_elements_without_Allocator(benchmark::State& state) {

  try {
    for (auto _ : state) {
      state.PauseTiming();
      const auto randomValue = Eigen::Vector4f::Random();
      jm::dynamic_circular_buffer<Eigen::Vector4f> data(state.range(0), randomValue);
      state.ResumeTiming();

      std::for_each(data.begin(), data.end(), [](auto& value) {
        const auto randomValue = Eigen::Vector4f::Random();
        benchmark::DoNotOptimize(value.dot(randomValue));
      });
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void BM_DynamicCircleBufferEigen_1K_elements_with_Allocator(benchmark::State& state) {

  try {
    for (auto _ : state) {
      state.PauseTiming();
      const auto randomValue = Eigen::Vector4f::Random();
      jm::dynamic_circular_buffer<Eigen::Vector4f, Eigen::aligned_allocator< jm::detail::optional_storage<Eigen::Vector4f> >> data(state.range(0), randomValue);
      state.ResumeTiming();

      std::for_each(data.begin(), data.end(), [](auto& value) {
        const auto randomValue = Eigen::Vector4f::Random();
        benchmark::DoNotOptimize(value.dot(randomValue));
      });
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

//Register the function as a benchmark
BENCHMARK(BM_StaticCircleBufferCreation_k1kB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1MB);

BENCHMARK(BM_StaticCircleBufferCreation_k1kB_push_back)->RangeMultiplier(2)->Range(128, 8<<8);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB_push_back)->RangeMultiplier(2)->Range(128, 8<<8);

BENCHMARK(BM_StaticCircleBufferCreation_k1kB_iteration)->RangeMultiplier(2)->Range(128, 8<<8);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB_iteration)->RangeMultiplier(2)->Range(128, 8<<8);

BENCHMARK(BM_DynamicCircleBufferEigen_1K_elements_without_Allocator)->RangeMultiplier(2)->Range(128, 8<<8);
BENCHMARK(BM_STDVectorEigen_1K_elements_without_Allocator)->RangeMultiplier(2)->Range(128, 8<<8);


BENCHMARK(BM_DynamicCircleBufferEigen_1K_elements_with_Allocator)->RangeMultiplier(2)->Range(128, 8<<8);
BENCHMARK(BM_STDVectorEigen_1K_elements_with_Allocator)->RangeMultiplier(2)->Range(128, 8<<8);



BENCHMARK_MAIN();



