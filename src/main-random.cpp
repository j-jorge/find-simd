#include "find.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdio>
#include <chrono>
#include <numeric>
#include <random>
#include <thread>

#include <benchmark/benchmark.h>

static std::vector<int> build_ordered_vector(size_t n)
{
  std::vector<int> values(n);
  std::iota(values.begin(), values.end(), 1);

  return values;
}

#define input_size                              \
  Arg(1 << 2)                                   \
  ->Arg(1 << 4)                                 \
  ->Arg(1 << 8)                                 \
  ->Arg(1 << 9)                                 \
  ->Arg(1 << 10)                                \
  ->Arg(1 << 11)                                \
  ->Arg(1 << 12)                                \
  ->Arg(1 << 13)                                \
  ->Arg(1 << 14)                                \
  ->Arg(1 << 15)                                \
  ->Arg(1 << 16)                                \
  ->Arg(1 << 17)                                \
  ->Arg(1 << 18)                                \
  ->Arg(1 << 19)                                \
  ->Arg(1 << 20)                                \
  ->Arg(1 << 21)                                \
  ->Arg(1 << 22)                                \
  ->Arg(1 << 23)                                \
  ->Arg(1 << 24)                                \

#define declare_benchmark(impl)                                         \
  static void benchmark_ ## impl(benchmark::State& state)               \
  {                                                                     \
    const size_t n = state.range(0);                                    \
    const std::vector<int> values = build_ordered_vector(n);            \
    std::mt19937 random(n);                                             \
    std::uniform_int_distribution<int> distribution(0, n-1);            \
                                                                        \
    for (auto _ : state)                                                \
      benchmark::DoNotOptimize                                          \
        (find_int_ ## impl(distribution(random), values.data(), n));    \
  }                                                                     \
                                                                        \
  BENCHMARK(benchmark_ ## impl)->input_size

declare_benchmark(c);
declare_benchmark(c_unrolled_8);
declare_benchmark(cpp);
declare_benchmark(sse2);

static std::chrono::nanoseconds now()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::steady_clock::now().time_since_epoch());
}

static int measure(const std::vector<int>& values, int k)
{
  const size_t n = values.size();

  const std::chrono::nanoseconds start = now();
  const size_t p_c = find_int_c(k, values.data(), n);

  const std::chrono::nanoseconds end_c = now();
  const size_t p_c_unrolled =
    find_int_c_unrolled_8(k, values.data(), n);

  const std::chrono::nanoseconds end_c_unrolled = now();
  const size_t p_cpp = find_int_cpp(k, values.data(), n);

  const std::chrono::nanoseconds end_cpp = now();
  const size_t p_sse2 = find_int_sse2(k, values.data(), n) ;

  const std::chrono::nanoseconds end_sse2 = now();

  const char* names[] =
    {
      "C",
      "C unrolled",
      "C++",
      "SSE2"
    };
  const size_t durations[] =
    {
      (size_t)(end_c - start).count(),
      (size_t)(end_c_unrolled - end_c).count(),
      (size_t)(end_cpp - end_c_unrolled).count(),
      (size_t)(end_sse2 - end_cpp).count()
    };

  int winner = 0;

  for (int i = 0; i != 4; ++i)
    if (durations[i] < durations[winner])
        winner = i;

  for (int i = 0; i != 4; ++i)
    std::cout << names[i] << ": " << durations[i] << " ns., ";

  std::cout << "winner is " << names[winner] << ".\n";

  if (p_c_unrolled != p_sse2)
    {
      std::cerr << "Mismatch: c unrolled -> " << p_c_unrolled << ", sse2 -> "
                << p_sse2
                << '\n';
      return 1;
    }

  if (p_cpp != p_sse2)
    {
      std::cerr << "Mismatch: c++ -> " << p_cpp << ", sse2 -> " << p_sse2
                << '\n';
      return 1;
    }

  if (p_c != p_sse2)
    {
      std::cerr << "Mismatch: c -> " << p_c << ", sse2 -> " << p_sse2
                << '\n';
      return 1;
    }

  return 0;
}

static int measure(int n)
{
  std::vector<int> values(n);
  std::iota(values.begin(), values.end(), 1);

  int r = 0;
  std::mt19937 random(n);                                             \
  std::uniform_int_distribution<int> distribution(0, n-1);            \

  std::cout << "CTRL+C to exit.\n";

  while (true)
    {
      r |= measure(values, distribution(random));
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char** argv)
{
  if (argc == 2)
    {
      const int n = atoi(argv[1]);

      if (n > 0)
        return measure(n);
    }

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
}
