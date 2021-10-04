#include "find.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdio>
#include <chrono>
#include <numeric>

#include <benchmark/benchmark.h>

static std::vector<int> build_ordered_vector(size_t n)
{
  std::vector<int> values(n);
  std::iota(values.begin(), values.end(), 1);

  return values;
}

#define input_size                              \
  Arg(1 << 8)                                   \
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

#define declare_benchmark(impl)                                 \
  static void benchmark_ ## impl(benchmark::State& state)       \
  {                                                             \
    const size_t n = state.range(0);                            \
    const std::vector<int> values = build_ordered_vector(n);    \
                                                                \
    for (auto _ : state)                                        \
      benchmark::DoNotOptimize                                  \
        (find_int_ ## impl(values.back(), values.data(), n));   \
  }                                                             \
                                                                \
  BENCHMARK(benchmark_ ## impl)->input_size

declare_benchmark(c);
declare_benchmark(c_unrolled_8);
declare_benchmark(cpp);
declare_benchmark(sse2);

template<typename F>
static int test_one(F&& f, int expected, int k, const int* v, size_t n)
{
  const size_t r = f(k, v, n);

  if (r == expected)
    return 0;

  printf
    ("FAIL:\n%s\nexpected: %d\ngot: %d\nk=%d\nv={",
     __PRETTY_FUNCTION__, expected, (int)r, k);

  for (int i = 0; i < n - 1; ++i)
    printf(" %d,", v[i]);

  if (n != 0)
    printf(" %d }\n", v[n - 1]);
  else
    printf(" }\n");

  return 1;
}

template<size_t N>
static int test_all(int expected, int k, const int (&v)[N])
{
  return
    test_one(find_int_c, expected, k, v, N)
    + test_one(find_int_c_unrolled_8, expected, k, v, N)
    + test_one(find_int_cpp, expected, k, v, N)
    + test_one(find_int_sse2, expected, k, v, N);
}

static int test()
{
  int r = test_one(find_int_c, 0, 42, nullptr, 0)
    + test_one(find_int_c_unrolled_8, 0, 42, nullptr, 0)
    + test_one(find_int_cpp, 0, 42, nullptr, 0)
    + test_one(find_int_sse2, 0, 42, nullptr, 0);

  r += test_all(0, 42, {42});

  r += test_all(1, 42, {31, 42});
  r += test_all(0, 42, {42, 31});
  r += test_all(0, 42, {42, 42});

  r += test_all(2, 42, {68, 31, 42});
  r += test_all(1, 42, {68, 42, 31});
  r += test_all(0, 42, {42, 68, 31});
  r += test_all(0, 42, {42, 68, 42});
  r += test_all(0, 42, {42, 42, 42});

  r += test_all(3, 42, {5, 68, 31, 42});
  r += test_all(2, 42, {5, 68, 42, 31});
  r += test_all(1, 42, {5, 42, 68, 31});
  r += test_all(0, 42, {42, 5, 68, 31});
  r += test_all(0, 42, {42, 5, 68, 42});
  r += test_all(0, 42, {42, 5, 42, 42});
  r += test_all(0, 42, {42, 42, 42, 42});

  r += test_all(0, 4, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(1, 8, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(2, 16, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(3, 32, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(4, 64, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(5, 128, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(6, 256, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(7, 512, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(8, 1024, {4, 8, 16, 32, 64, 128, 256, 512, 1024});
  r += test_all(9, 2048, {4, 8, 16, 32, 64, 128, 256, 512, 1024});

  return r;
}

static std::chrono::milliseconds now()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::steady_clock::now().time_since_epoch());
}

static int measure(int n)
{

  std::vector<int> values(n);
  std::iota(values.begin(), values.end(), 1);

  const std::chrono::milliseconds start = now();
  const size_t p_c = find_int_c(values.back(), values.data(), n);

  const std::chrono::milliseconds end_c = now();
  const size_t p_c_unrolled =
    find_int_c_unrolled_8(values.back(), values.data(), n);

  const std::chrono::milliseconds end_c_unrolled = now();
  const size_t p_cpp = find_int_cpp(values.back(), values.data(), n);

  const std::chrono::milliseconds end_cpp = now();
  const size_t p_sse2 = find_int_sse2(values.back(), values.data(), n) ;

  const std::chrono::milliseconds end_sse2 = now();

  std::cout << "C: " << (end_c - start).count() << " ms., "
            << "C unrolled: " << (end_c_unrolled - end_c).count() << " ms., "
            << "C++: " << (end_cpp - end_c_unrolled).count() << " ms., "
            << "SSE2: " << (end_sse2 - end_cpp).count() << " ms.\n";

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

int main(int argc, char** argv)
{
  if (test() != 0)
    return 1;

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
