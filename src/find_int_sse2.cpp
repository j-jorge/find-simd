#include <cstdint>
#include <immintrin.h>

size_t find_int_sse2(int k, const int* v, size_t n)
{
  // We are going to check the integers from v four by four.

  // This instruction copies the searched value k in each of the four 32-bits
  // lanes of the 128 bits register. If k…k represents the 32 bits of k, the
  // register then looks like this:
  //
  // [ k…k | k…k | k…k | k…k ]
  //
  const __m128i needle = _mm_set1_epi32(k);

  // Here we just reinterpret the pointer v as a pointer to 128 bits
  // vectors. This will allow to access the integers four by four. And yes, I do
  // use a C cast like a barbarian.
  const __m128i* p = (const __m128i*)v;

  // A division! Will the compile emit a shift for it?
  const size_t iterations = n / 4;
  const size_t tail = n % 4;

  for (size_t i(0); i != iterations; ++i, ++p)
    {
      const __m128i haystack = *p;

      // This compares all four 32 bits values of needle (so four times the
      // searched value) with four integers pointed by p, all at once. All bits
      // of a 32 bits lane are set to 1 if the values are equal. The operation
      // looks like this:
      //
      //       [ k…k | k…k | k…k | k…k ]
      // cmpeq [ a…a | b…b | k…k | c…c ]
      //     = [ 0…0 | 0…0 | 1…1 | 0…0 ]
      //
      const __m128i mask = _mm_cmpeq_epi32(needle, haystack);

      // We cannot compare mask against zero directly but we have an instruction
      // to help us check the bits in mask.

      // This instruction takes the most significant bit of each of the sixteen
      // 8 bits lanes from a 128 bits register and put them in the low bits of
      // a 32 bits integer.
      //
      // Consequently, since there are four 8-bits lanes in an integer, if k has
      // been found in haystack we should have a 0b1111 in eq. Otherwise eq will
      // be zero. For example:
      //
      // movemask                     [ 0…0 | 0…0 | 1…1 | 0…0 ]
      //        = 0000 0000 0000 0000  0000  0000  1111  0000
      //
      const uint32_t eq = _mm_movemask_epi8(mask);

      // Since we are back with a scalar value, we can test it directly.
      if (eq == 0)
        continue;

      // Now we just have to find the offset of the lowest bit set to 1. My
      // laptop does not support the tzcnt instruction so I use the GCC builtin
      // equivalent.
      const unsigned zero_bits_count = __builtin_ctz(eq);

      // Each 4 bits group in eq matches a 32 bits value from mask, thus we
      // divide by 4.
      return i * 4 + zero_bits_count / 4;
    }

  // Handle the last entries if the number of entries was not a multiple of
  // four.
  for (size_t i(iterations * 4); i != n; ++i)
    if (v[i] == k)
      return i;

  return n;
}
