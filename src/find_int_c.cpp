#include <cstddef>

size_t find_int_c(int k, const int* v, size_t n)
{
  for (size_t i = 0; i != n; ++i)
    if (v[i] == k)
      return i;

  return n;
}
