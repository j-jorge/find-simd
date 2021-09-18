#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdio>
#include <chrono>
#include <numeric>

#include <immintrin.h>

#include <benchmark/benchmark.h>

size_t find_int_c(int k, const int* v, size_t n)
{
  for (size_t i = 0; i != n; ++i)
    if (v[i] == k)
      return i;

  return n;
}

size_t find_int_c_unrolled_8(int k, const int* v, size_t n)
{
  size_t i = 0;

  for (; n - i >= 8; i += 8)
    {
      if (v[i] == k)
        return i;
      if (v[i+1] == k)
        return i + 1;
      if (v[i+2] == k)
        return i + 2;
      if (v[i+3] == k)
        return i + 3;
      if (v[i+4] == k)
        return i + 4;
      if (v[i+5] == k)
        return i + 5;
      if (v[i+6] == k)
        return i + 6;
      if (v[i+7] == k)
        return i + 7;
    }

  do
    {
      if (v[i] == k)
        return i;

      ++i;
    }
  while(i != n);

  return n;
}

size_t find_int_cpp(int k, const int* v, size_t n)
{
  return std::find(v, v + n, k) - v;
}

size_t find_int_sse2(int k, const int* v, size_t n)
{
  // We are going to check the integers from v four by four.

  // This copies the searched value k in each of the four 32-bits lanes of the
  // _128i register.
  const __m128i needle = _mm_set1_epi32(k);
  const __m128i* p = (const __m128i*)v;

  for (size_t i(0); i < n; i += 4, ++p)
    {
      const __m128i haystack = *p;

      // This compares all four values of needle (so four times the searched
      // value) with four integers pointed by p, all at once. All bits of a 32
      // bits lane are set to 1 if the values are equal.
      const __m128i mask = _mm_cmpeq_epi32(needle, haystack);

      // We cannot compare mask against zero directly but we have an instruction
      // to help us check the bits in mask.

      // This instruction takes the most significant bit of each of the sixteen
      // 8 bits lanes from a 128 bits register and put them in the low bits of
      // an integer.
      //
      // Consequently, since there are four 8-bits lanes in an integer, if k has
      // been found in haystack we should have a 0b1111 in eq.
      const uint32_t eq = _mm_movemask_epi8(mask);

      if (eq == 0)
        continue;

      // Now we just have to find the offset of the lowest bit set to 1. My
      // laptop does not support the tzcnt instruction so I use the GCC builtin
      // equivalent.
      const unsigned zero_bits_count = __builtin_ctz(eq);

      // Since n may not be a multiple of 4, we may have found the value outside
      // the allowed range, thus we need to adjust the index accordingly.

      // A division! Will the compile emit a shift for it?
      return std::min(i + zero_bits_count / 4, n);
    }

  return n;
}

std::vector<int> build_ordered_vector(size_t n)
{
  std::vector<int> values(n);
  std::iota(values.begin(), values.end(), 1);

  return values;
}

static void benchmark_c(benchmark::State& state)
{
  const size_t n = state.range(0);
  const std::vector<int> values = build_ordered_vector(n);

  for (auto _ : state)
    benchmark::DoNotOptimize(find_int_c(values.back(), values.data(), n));
}

static void benchmark_c_unrolled_8(benchmark::State& state)
{
  const size_t n = state.range(0);
  const std::vector<int> values = build_ordered_vector(n);

  for (auto _ : state)
    benchmark::DoNotOptimize
      (find_int_c_unrolled_8(values.back(), values.data(), n));
}

static void benchmark_cpp(benchmark::State& state)
{
  const size_t n = state.range(0);
  const std::vector<int> values = build_ordered_vector(n);

  for (auto _ : state)
    benchmark::DoNotOptimize(find_int_cpp(values.back(), values.data(), n));
}

static void benchmark_sse2(benchmark::State& state)
{
  const size_t n = state.range(0);
  const std::vector<int> values = build_ordered_vector(n);

  for (auto _ : state)
    benchmark::DoNotOptimize(find_int_sse2(values.back(), values.data(), n));
}

#define RANGE Range(10, 10000000)

BENCHMARK(benchmark_c)->RANGE;
BENCHMARK(benchmark_c_unrolled_8)->RANGE;
BENCHMARK(benchmark_cpp)->RANGE;
BENCHMARK(benchmark_sse2)->RANGE;

std::chrono::milliseconds now()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::steady_clock::now().time_since_epoch());
}

int main(int argc, char** argv)
{
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return 0;
  const int n = atoi(argv[1]);
  std::vector<int> values(n);
  std::iota(values.begin(), values.end(), 1);

  const std::chrono::milliseconds start = now();
  const size_t p_c = find_int_c(values.back(), values.data(), n);

  const std::chrono::milliseconds end_c = now();
  const size_t p_cpp = find_int_cpp(values.back(), values.data(), n);

  const std::chrono::milliseconds end_cpp = now();
  const size_t p_sse2 = find_int_sse2(values.back(), values.data(), n) ;

  const std::chrono::milliseconds end_sse2 = now();

  std::cout << "C: " << (end_c - start).count() << " ms., "
            << "C++: " << (end_cpp - end_c).count() << " ms., "
            << "SSE2: " << (end_sse2 - end_cpp).count() << " ms.\n";

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
