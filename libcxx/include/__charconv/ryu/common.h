//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Copyright 2018 Ulf Adams
// Copyright (c) Microsoft Corporation. All rights reserved.

// Boost Software License - Version 1.0 - August 17th, 2003

#ifndef _LIBCPP___CHARCONV_RYU_COMMON_H
#define _LIBCPP___CHARCONV_RYU_COMMON_H

// Avoid formatting to keep the changes with the original code minimal.
// clang-format off

#include <__assert>
#include <__bit/bit_cast.h>
#include <__config>
#include <cstddef>
#include <cstdint>

#if defined(_WIN64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__aarch64__)
#define _LIBCPP_64_BIT
#endif

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

// Shared with the out-of-line implementation in libc++/src/ryu. The constants
// live here (instead of in d2s.h) so that d2fixed.h can use them without
// including d2s.h.
inline constexpr int __DOUBLE_MANTISSA_BITS = 52;
inline constexpr int __DOUBLE_EXPONENT_BITS = 11;
inline constexpr int __DOUBLE_BIAS = 1023;

inline constexpr int __DOUBLE_POW5_INV_BITCOUNT = 122;
inline constexpr int __DOUBLE_POW5_BITCOUNT = 121;

inline constexpr int __POW10_ADDITIONAL_BITS = 120;

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline uint32_t __decimalLength9(const uint32_t __v) {
  // Function precondition: __v is not a 10-digit number.
  // (f2s: 9 digits are sufficient for round-tripping.)
  // (d2fixed: We print 9-digit blocks.)
  _LIBCPP_ASSERT_INTERNAL(__v < 1000000000, "");
  if (__v >= 100000000) { return 9; }
  if (__v >= 10000000) { return 8; }
  if (__v >= 1000000) { return 7; }
  if (__v >= 100000) { return 6; }
  if (__v >= 10000) { return 5; }
  if (__v >= 1000) { return 4; }
  if (__v >= 100) { return 3; }
  if (__v >= 10) { return 2; }
  return 1;
}

// Returns __e == 0 ? 1 : ceil(log_2(5^__e)).
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline int32_t __pow5bits(const int32_t __e) {
  // This approximation works up to the point that the multiplication overflows at __e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  _LIBCPP_ASSERT_INTERNAL(__e >= 0, "");
  _LIBCPP_ASSERT_INTERNAL(__e <= 3528, "");
  return static_cast<int32_t>(((static_cast<uint32_t>(__e) * 1217359) >> 19) + 1);
}

// Returns floor(log_10(2^__e)).
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline uint32_t __log10Pow2(const int32_t __e) {
  // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
  _LIBCPP_ASSERT_INTERNAL(__e >= 0, "");
  _LIBCPP_ASSERT_INTERNAL(__e <= 1650, "");
  return (static_cast<uint32_t>(__e) * 78913) >> 18;
}

// Returns floor(log_10(5^__e)).
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline uint32_t __log10Pow5(const int32_t __e) {
  // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
  _LIBCPP_ASSERT_INTERNAL(__e >= 0, "");
  _LIBCPP_ASSERT_INTERNAL(__e <= 2620, "");
  return (static_cast<uint32_t>(__e) * 732923) >> 20;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline uint32_t __float_to_bits(const float __f) {
  return std::__bit_cast<uint32_t>(__f);
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline uint64_t __double_to_bits(const double __d) {
  return std::__bit_cast<uint64_t>(__d);
}

// The original code uses the C library functions memcpy, memset and memmove.
// These are not usable in a constant expression, so they are replaced by
// small equivalent loops.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline char*
__ryu_memcpy(char* __dst, const char* __src, size_t __n) {
  for (size_t __i = 0; __i != __n; ++__i)
    __dst[__i] = __src[__i];
  return __dst;
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline char*
__ryu_memset(char* __dst, int __c, size_t __n) {
  for (size_t __i = 0; __i != __n; ++__i)
    __dst[__i] = static_cast<char>(__c);
  return __dst;
}

// Precondition: __dst <= __src.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline char*
__ryu_memmove(char* __dst, const char* __src, size_t __n) {
  for (size_t __i = 0; __i != __n; ++__i)
    __dst[__i] = __src[__i];
  return __dst;
}

#if !defined(_MSC_VER)
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline unsigned char
_BitScanForward64(unsigned long* __index, unsigned long long __mask) {
  if (__mask == 0) {
    return false;
  }
  *__index = __builtin_ctzll(__mask);
  return true;
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline unsigned char
_BitScanForward(unsigned long* __index, unsigned int __mask) {
  if (__mask == 0) {
    return false;
  }
  *__index = __builtin_ctz(__mask);
  return true;
}
#endif // !_MSC_VER

_LIBCPP_END_NAMESPACE_STD

// clang-format on

#endif // _LIBCPP___CHARCONV_RYU_COMMON_H
