#include <cstddef>

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
