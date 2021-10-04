#include <algorithm>
#include <cstdint>

size_t find_int_cpp(int k, const int* v, size_t n)
{
  return std::find(v, v + n, k) - v;
}
