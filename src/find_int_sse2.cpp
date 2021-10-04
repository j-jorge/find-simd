#include <cstdint>
#include <immintrin.h>

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
