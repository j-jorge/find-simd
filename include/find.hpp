#pragma once

#include <cstddef>

size_t find_int_c(int k, const int* v, size_t n);
size_t find_int_c_unrolled_8(int k, const int* v, size_t n);
size_t find_int_cpp(int k, const int* v, size_t n);
size_t find_int_sse2(int k, const int* v, size_t n);
