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

  for (; i != n; ++i)
    if (v[i] == k)
      return i;

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
  // __m128i register.
  const __m128i needle = _mm_set1_epi32(k);
  const __m128i* p = (const __m128i*)v;

  // A division! Will the compile emit a shift for it?
  const size_t iterations = n / 4;
  const size_t tail = n % 4;

  for (size_t i(0); i != iterations; ++i, ++p)
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

      return i * 4 + zero_bits_count / 4;
    }

  // Handle the last entries if the number of entries was not a multiple of
  // four.
  for (size_t i(iterations * 4); i != n; ++i)
    if (v[i] == k)
      return i;

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

#define VALUES                                  \
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

BENCHMARK(benchmark_c)->VALUES;
BENCHMARK(benchmark_c_unrolled_8)->VALUES;
BENCHMARK(benchmark_cpp)->VALUES;
BENCHMARK(benchmark_sse2)->VALUES;

template<typename F>
int test_one(F&& f, int expected, int k, const int* v, size_t n)
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
int test_all(int expected, int k, const int (&v)[N])
{
  return
    test_one(find_int_c, expected, k, v, N)
    + test_one(find_int_c_unrolled_8, expected, k, v, N)
    + test_one(find_int_cpp, expected, k, v, N)
    + test_one(find_int_sse2, expected, k, v, N);
}

int test()
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

std::chrono::milliseconds now()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::steady_clock::now().time_since_epoch());
}

int main(int argc, char** argv)
{
  if (test() != 0)
    return 1;

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
