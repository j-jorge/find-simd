#include "find.hpp"

#include <cstdio>

template<typename F>
static int test_one
(const char* name, F&& f, int expected, int k, const int* v, size_t n)
{
  const size_t r = f(k, v, n);

  if (r == expected)
    return 0;

  printf
    ("FAIL:\n%s\nexpected: %d\ngot: %d\nk=%d\nv={",
     name, expected, (int)r, k);

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
    test_one("c", find_int_c, expected, k, v, N)
    + test_one("c_unrolled_8", find_int_c_unrolled_8, expected, k, v, N)
    + test_one("cpp", find_int_cpp, expected, k, v, N)
    + test_one("sse2", find_int_sse2, expected, k, v, N);
}

static int test()
{
  int r = test_one("c", find_int_c, 0, 42, nullptr, 0)
    + test_one("c_unrolled", find_int_c_unrolled_8, 0, 42, nullptr, 0)
    + test_one("cpp", find_int_cpp, 0, 42, nullptr, 0)
    + test_one("sse2", find_int_sse2, 0, 42, nullptr, 0);

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

int main(int argc, char** argv)
{
  return test();
}
