// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_IMPL_H
#define _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_IMPL_H

// Avoid formatting to keep the changes with the original code minimal.
// clang-format off

#include <__assert>
#include <__bit/bit_cast.h>
#include <__charconv/chars_format.h>
#include <__charconv/from_chars_result.h>
#include <__config>
#include <__system_error/errc.h>
#include <__type_traits/is_constant_evaluated.h>
#include <cstddef>
#include <cstdint>
#include <limits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 17

template <class _Fp>
struct __from_chars_result {
  _Fp __value;
  ptrdiff_t __n;
  errc __ec;
};

// A fixed-size unsigned integer, used to do the exact arithmetic needed to
// round a decimal or hexadecimal literal to the nearest floating-point value.
// This is only used for constant evaluation; at run time the (faster) library
// implementation is called instead. 4096 bits are enough for every value of
// float and double, including a full-precision significand (see
// __max_mantissa_bits) scaled by a clamped exponent.
struct __fc_big_int {
  static constexpr int __capacity = 128; // 128 * 32 = 4096 bits
  static constexpr int __max_exp10 = 800;

  // Number of significand digits that are accumulated exactly. 1500 bits are
  // far more than the ~1450 bits needed to make the contribution of truncated
  // digits smaller than one unit in the rounding step, which in turn makes the
  // single "truncated" (sticky) flag sufficient for correct rounding.
  static constexpr int __max_mantissa_bits = 1500;

  uint32_t __d[__capacity];
  int __n; // number of significant words, 0 for zero

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __zero() {
    for (int __i = 0; __i != __capacity; ++__i)
      __d[__i] = 0;
    __n = 0;
  }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __set(uint64_t __v) {
    __zero();
    __d[0] = static_cast<uint32_t>(__v);
    __d[1] = static_cast<uint32_t>(__v >> 32);
    __n     = __d[1] != 0 ? 2 : (__d[0] != 0 ? 1 : 0);
  }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 bool __is_zero() const { return __n == 0; }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 int __bit_length() const {
    if (__n == 0)
      return 0;
    uint32_t __top = __d[__n - 1];
    int __bits     = 0;
    while (__top != 0) {
      ++__bits;
      __top >>= 1;
    }
    return (__n - 1) * 32 + __bits;
  }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __mul_small(uint32_t __m) {
    uint64_t __carry = 0;
    for (int __i = 0; __i != __n; ++__i) {
      const uint64_t __t = static_cast<uint64_t>(__d[__i]) * __m + __carry;
      __d[__i]           = static_cast<uint32_t>(__t);
      __carry            = __t >> 32;
    }
    if (__carry != 0) {
      __d[__n] = static_cast<uint32_t>(__carry);
      ++__n;
    }
  }

  // Requires the result to fit in __capacity words.
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __add_small(uint32_t __v) {
    uint64_t __carry = __v;
    for (int __i = 0; __i != __n && __carry != 0; ++__i) {
      const uint64_t __t = static_cast<uint64_t>(__d[__i]) + __carry;
      __d[__i]           = static_cast<uint32_t>(__t);
      __carry            = __t >> 32;
    }
    if (__carry != 0) {
      __d[__n] = static_cast<uint32_t>(__carry);
      ++__n;
    }
  }

  // Requires __n < __capacity.
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __add(const __fc_big_int& __other) {
    uint64_t __carry = 0;
    const int __len  = __n > __other.__n ? __n : __other.__n;
    for (int __i = 0; __i != __len; ++__i) {
      const uint64_t __t = static_cast<uint64_t>(__d[__i]) + (__i < __other.__n ? __other.__d[__i] : 0) + __carry;
      __d[__i]           = static_cast<uint32_t>(__t);
      __carry            = __t >> 32;
    }
    __n = __len;
    if (__carry != 0) {
      __d[__n] = static_cast<uint32_t>(__carry);
      ++__n;
    }
  }

  // Requires *this >= __other.
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __sub(const __fc_big_int& __other) {
    int64_t __borrow = 0;
    for (int __i = 0; __i != __n; ++__i) {
      int64_t __t = static_cast<int64_t>(__d[__i]) - (__i < __other.__n ? __other.__d[__i] : 0) - __borrow;
      if (__t < 0) {
        __t += (int64_t(1) << 32);
        __borrow = 1;
      } else {
        __borrow = 0;
      }
      __d[__i] = static_cast<uint32_t>(__t);
    }
    while (__n != 0 && __d[__n - 1] == 0)
      --__n;
  }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __shift_left(int __bits) {
    if (__n == 0 || __bits == 0)
      return;
    const int __w = __bits / 32;
    const int __b = __bits % 32;
    if (__w != 0) {
      for (int __i = __n - 1; __i >= 0; --__i)
        __d[__i + __w] = __d[__i];
      for (int __i = 0; __i != __w; ++__i)
        __d[__i] = 0;
      __n += __w;
    }
    if (__b != 0) {
      uint32_t __carry = 0;
      for (int __i = __w; __i != __n; ++__i) {
        const uint64_t __t = (static_cast<uint64_t>(__d[__i]) << __b) | __carry;
        __d[__i]           = static_cast<uint32_t>(__t);
        __carry            = static_cast<uint32_t>(__t >> 32);
      }
      if (__carry != 0) {
        __d[__n] = __carry;
        ++__n;
      }
    }
    while (__n != 0 && __d[__n - 1] == 0)
      --__n;
  }

  // Discards the low __bits bits.
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __shift_right(int __bits) {
    if (__n == 0)
      return;
    const int __w = __bits / 32;
    const int __b = __bits % 32;
    if (__w >= __n) {
      __n = 0;
      return;
    }
    for (int __i = 0; __i != __n - __w; ++__i) {
      uint32_t __lo = __d[__i + __w] >> __b;
      if (__b != 0 && __i + __w + 1 < __n)
        __lo |= __d[__i + __w + 1] << (32 - __b);
      __d[__i] = __lo;
    }
    __n -= __w;
    while (__n != 0 && __d[__n - 1] == 0)
      --__n;
  }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 int __compare(const __fc_big_int& __other) const {
    if (__n != __other.__n)
      return __n < __other.__n ? -1 : 1;
    for (int __i = __n - 1; __i >= 0; --__i)
      if (__d[__i] != __other.__d[__i])
        return __d[__i] < __other.__d[__i] ? -1 : 1;
    return 0;
  }

  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 uint64_t __low64() const {
    uint64_t __r = __d[0];
    if (__n > 1)
      __r |= static_cast<uint64_t>(__d[1]) << 32;
    return __r;
  }

  // The value is 5^__e.
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 static __fc_big_int __power_of_five(int __e) {
    __fc_big_int __r;
    __r.__set(1);
    for (int __i = 0; __i != __e; ++__i)
      __r.__mul_small(5);
    return __r;
  }

  // Multiplies by 10^__e. Requires __e <= __max_exp10.
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 void __mul_pow10(int __e) {
    // Multiplying by 10^9 at a time keeps the number of passes small.
    while (__e >= 9) {
      __mul_small(1000000000u);
      __e -= 9;
    }
    uint32_t __m = 1;
    for (int __i = 0; __i != __e; ++__i)
      __m *= 10;
    if (__m != 1)
      __mul_small(__m);
  }
};

template <class _Fp>
struct __fc_traits;

template <>
struct __fc_traits<float> {
  static constexpr int __p        = 24;
  static constexpr int __emin     = -126;
  static constexpr int __ebits    = 8;
  static constexpr int __bias     = 127;
  static constexpr int __max_biased = 254; // 0xFE, the largest finite biased exponent
  static constexpr int __max_dec  = 40;  // beyond this the value certainly overflows
  static constexpr int __min_dec  = -520; // below this it certainly underflows to zero
  static constexpr int __max_bin  = 130;  // same, for hexadecimal values
  static constexpr int __min_bin  = -1660;
};

template <>
struct __fc_traits<double> {
  static constexpr int __p        = 53;
  static constexpr int __emin     = -1022;
  static constexpr int __ebits    = 11;
  static constexpr int __bias     = 1023;
  static constexpr int __max_biased = 2046; // 0x7FE
  static constexpr int __max_dec  = 320;
  static constexpr int __min_dec  = -800;
  static constexpr int __max_bin  = 1030;
  static constexpr int __min_bin  = -2585;
};

// Character classification helpers. std::tolower/std::isdigit are not usable in
// a constant expression, and std::from_chars is specified to use the "C"
// locale for the characters it recognizes, so plain ASCII comparisons are
// enough here.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline char __fc_to_lower(char __c) {
  return (__c >= 'A' && __c <= 'Z') ? static_cast<char>(__c - 'A' + 'a') : __c;
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline bool __fc_is_digit(char __c) {
  return __c >= '0' && __c <= '9';
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline bool __fc_is_xdigit(char __c) {
  return __fc_is_digit(__c) || (__c >= 'a' && __c <= 'f') || (__c >= 'A' && __c <= 'F');
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline unsigned __fc_xdigit_value(char __c) {
  if (__fc_is_digit(__c))
    return static_cast<unsigned>(__c - '0');
  return static_cast<unsigned>(__fc_to_lower(__c) - 'a' + 10);
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline bool __fc_is_alnum(char __c) {
  return __fc_is_digit(__c) || (__c >= 'a' && __c <= 'z') || (__c >= 'A' && __c <= 'Z');
}

// The result of parsing the significand of a floating-point value. The
// significand is accumulated in a big integer, which keeps enough digits for
// the later rounding step to be exact. Only digits that cannot influence the
// result any more are dropped, and __truncated is set when one of them is not
// zero. This "sticky" bit is then used when rounding.
struct __fc_fractional {
  ptrdiff_t __offset;
  __fc_big_int __mantissa;
  int __exponent;
  bool __truncated;
  bool __valid;
};

// Multiplies the significand by __base, adds __digit and adjusts __exponent,
// dropping the digit instead when the significand has reached full precision.
// Both bases used here are at most 16, so a digit adds at most 4 bits.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline void __fc_accumulate(
    __fc_big_int& __mantissa, int& __exponent, bool& __truncated, bool __fraction, uint32_t __base, uint32_t __digit,
    int __exponent_per_digit) {
  if (__mantissa.__bit_length() + 4 <= __fc_big_int::__max_mantissa_bits) {
    __mantissa.__mul_small(__base);
    __mantissa.__add_small(__digit);
    if (__fraction)
      __exponent -= __exponent_per_digit;
  } else {
    if (__digit != 0)
      __truncated = true;
    if (!__fraction)
      __exponent += __exponent_per_digit;
  }
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __fc_fractional
__fc_parse_decimal_fractional(const char* __input, ptrdiff_t __n, ptrdiff_t __offset) {
  __fc_fractional __result{__offset, __fc_big_int{}, 0, false, false};
  bool __fraction = false;
  for (; __offset < __n; ++__offset) {
    const char __c = __input[__offset];
    if (__fc_is_digit(__c)) {
      __result.__valid = true;
      const uint32_t __digit = static_cast<uint32_t>(__c - '0');
      __fc_accumulate(__result.__mantissa, __result.__exponent, __result.__truncated, __fraction, 10, __digit, 1);
    } else if (__c == '.') {
      if (__fraction)
        break; // a second decimal point ends the number
      __fraction = true;
    } else {
      break;
    }
  }
  __result.__offset = __offset;
  return __result;
}

_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __fc_fractional
__fc_parse_hex_fractional(const char* __input, ptrdiff_t __n, ptrdiff_t __offset) {
  __fc_fractional __result{__offset, __fc_big_int{}, 0, false, false};
  bool __fraction = false;
  for (; __offset < __n; ++__offset) {
    const char __c = __input[__offset];
    if (__fc_is_xdigit(__c)) {
      __result.__valid       = true;
      const uint32_t __digit = __fc_xdigit_value(__c);
      __fc_accumulate(__result.__mantissa, __result.__exponent, __result.__truncated, __fraction, 16, __digit, 4);
    } else if (__c == '.') {
      if (__fraction)
        break;
      __fraction = true;
    } else {
      break;
    }
  }
  __result.__offset = __offset;
  return __result;
}

struct __fc_exponent {
  ptrdiff_t __offset;
  int __value;
  bool __present;
};

// Parses the exponent part, marked by __marker ('e' for decimal values and 'p'
// for hexadecimal values). When no valid exponent is present the original
// offset is returned and __present is false, so the marker is not consumed.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __fc_exponent
__fc_parse_exponent(const char* __input, ptrdiff_t __n, ptrdiff_t __offset, char __marker) {
  if (__offset >= __n || __fc_to_lower(__input[__offset]) != __marker)
    return {__offset, 0, false};

  ptrdiff_t __i = __offset + 1;
  bool __negative = false;
  if (__i < __n && (__input[__i] == '+' || __input[__i] == '-')) {
    __negative = __input[__i] == '-';
    ++__i;
  }

  const ptrdiff_t __start = __i;
  int __value             = 0;
  for (; __i < __n && __fc_is_digit(__input[__i]); ++__i) {
    if (__value < 100000) // saturate, the value is clamped later anyway
      __value = __value * 10 + (__input[__i] - '0');
  }

  if (__i == __start)
    return {__offset, 0, false}; // the marker was not followed by an exponent

  return {__i, __negative ? -__value : __value, true};
}

// Adds the exponent that stems from the position of the decimal point to the
// explicit exponent, clamping the result. The clamping bounds are chosen such
// that the exact magnitude of a clamped value can no longer be represented by
// _Fp; this keeps the big integer arithmetic below within its capacity.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline int
__fc_merge_exponents(int64_t __fractional, int64_t __exponent, int __max, int __min) {
  const int64_t __sum = __fractional + __exponent;
  if (__sum > __max)
    return __max;
  if (__sum < __min)
    return __min;
  return static_cast<int>(__sum);
}

// Returns floor(log2(__n / __d)) for positive __n and __d.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline int
__fc_floor_log2_ratio(const __fc_big_int& __n, const __fc_big_int& __d) {
  int __e = __n.__bit_length() - __d.__bit_length();
  if (__e >= 0) {
    __fc_big_int __t = __d;
    __t.__shift_left(__e);
    if (__n.__compare(__t) < 0)
      --__e;
  } else {
    __fc_big_int __t = __n;
    __t.__shift_left(-__e);
    if (__t.__compare(__d) < 0)
      --__e;
  }
  return __e;
}

// Returns floor(__num / __den) and stores the remainder in __rem. The result is
// known to fit in 64 bits; it is at most 2^53 for the callers below.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline uint64_t
__fc_div_rem(const __fc_big_int& __num, const __fc_big_int& __den, __fc_big_int& __rem) {
  __rem = __num;
  int __shift = __num.__bit_length() - __den.__bit_length();
  if (__shift < 0)
    return 0;

  __fc_big_int __t = __den;
  __t.__shift_left(__shift);
  uint64_t __q = 0;
  for (int __i = __shift; __i >= 0; --__i) {
    if (__rem.__compare(__t) >= 0) {
      __rem.__sub(__t);
      __q |= uint64_t(1) << __i;
    }
    __t.__shift_right(1);
  }
  return __q;
}

// Round-half-to-even decision; __sticky means that the exact value is slightly
// larger than __rem / __den.
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline bool
__fc_should_round_up(const __fc_big_int& __rem, const __fc_big_int& __den, uint64_t __q, bool __sticky) {
  __fc_big_int __twice = __rem;
  __twice.__shift_left(1);
  const int __cmp = __twice.__compare(__den);
  if (__cmp != 0)
    return __cmp > 0;
  if (__sticky)
    return true;
  return (__q & 1) != 0;
}

template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline _Fp __fc_from_bits(uint64_t __bits) {
  if (sizeof(_Fp) == sizeof(uint32_t))
    return static_cast<_Fp>(__bit_cast<float>(static_cast<uint32_t>(__bits)));
  return static_cast<_Fp>(__bit_cast<double>(__bits));
}

// Assembles a finite floating-point value from the unbiased exponent __k of the
// least significant bit of the significand and the (rounded) significand __q.
// __q has at most __p bits. Sets __oor when the value is too large; values that
// round to zero are also reported as out of range.
template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline _Fp
__fc_encode(int __k, uint64_t __q, bool __negative, bool& __oor) {
  using __t = __fc_traits<_Fp>;
  constexpr int __p          = __t::__p;
  constexpr int __ebits      = __t::__ebits;
  constexpr uint64_t __sign  = uint64_t(1) << (__ebits + __p - 1);
  constexpr uint64_t __hidden = uint64_t(1) << (__p - 1);

  if (__q == 0) {
    __oor = true; // underflow
    return __fc_from_bits<_Fp>(__negative ? __sign : 0);
  }

  int __biased;
  uint64_t __mantissa;
  if (__q >= __hidden) {
    __biased   = __k + (__p - 1) + __t::__bias;
    __mantissa = __q - __hidden;
  } else {
    __biased   = 0; // subnormal
    __mantissa = __q;
  }

  if (__biased < 0 || static_cast<uint64_t>(__biased) > static_cast<uint64_t>(__t::__max_biased)) {
    __oor = true; // overflow
    return __fc_from_bits<_Fp>((__negative ? __sign : 0) |
                               (uint64_t(__t::__max_biased + 1) << (__p - 1)));
  }

  const uint64_t __bits = (uint64_t(__biased) << (__p - 1)) | __mantissa | (__negative ? __sign : 0);
  return __fc_from_bits<_Fp>(__bits);
}

// Rounds the exact positive value __n / __d (with __sticky saying whether the
// exact value is slightly larger) to the nearest value of _Fp.
template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline _Fp __fc_round_ratio(
    const __fc_big_int& __n, const __fc_big_int& __d, bool __sticky, bool __negative, bool& __oor) {
  using __t         = __fc_traits<_Fp>;
  constexpr int __p = __t::__p;

  const int __e = __fc_floor_log2_ratio(__n, __d);
  int __k       = (__e >= __t::__emin ? __e : __t::__emin) - (__p - 1);

  __fc_big_int __num = __n;
  __fc_big_int __den = __d;
  if (__k > 0)
    __den.__shift_left(__k);
  else if (__k < 0)
    __num.__shift_left(-__k);

  __fc_big_int __rem;
  uint64_t __q = __fc_div_rem(__num, __den, __rem);
  if (__fc_should_round_up(__rem, __den, __q, __sticky))
    ++__q;

  // Rounding may carry into the next binade.
  if (__q == (uint64_t(1) << __p)) {
    __q >>= 1;
    ++__k;
  }

  return __fc_encode<_Fp>(__k, __q, __negative, __oor);
}

// Parses an "inf" or "infinity" string, case insensitively. __ptr points past
// the leading 'i'.
template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __from_chars_result<_Fp>
__fc_from_chars_inf(const char* __first, const char* __last, const char* __ptr, bool __negative) {
  if (__last - __ptr < 2)
    return {_Fp{0}, 0, errc::invalid_argument};

  if (__fc_to_lower(__ptr[0]) != 'n' || __fc_to_lower(__ptr[1]) != 'f')
    return {_Fp{0}, 0, errc::invalid_argument};

  __ptr += 2;

  // The result is valid at this point; "inity" is consumed when present, so
  // that e.g. "INFINITZ" consumes "INF" only.
  if (__last - __ptr >= 5 && __fc_to_lower(__ptr[0]) == 'i' && __fc_to_lower(__ptr[1]) == 'n' &&
      __fc_to_lower(__ptr[2]) == 'i' && __fc_to_lower(__ptr[3]) == 't' && __fc_to_lower(__ptr[4]) == 'y')
    __ptr += 5;

  if (numeric_limits<_Fp>::has_infinity) {
    const _Fp __value = numeric_limits<_Fp>::infinity();
    return {__negative ? -__value : __value, __ptr - __first, errc{}};
  }
  return {_Fp{0}, __ptr - __first, errc::result_out_of_range};
}

// Parses a "nan" string, case insensitively. __ptr points past the leading 'n'.
template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __from_chars_result<_Fp>
__fc_from_chars_nan(const char* __first, const char* __last, const char* __ptr, bool __negative) {
  if (__last - __ptr < 2)
    return {_Fp{0}, 0, errc::invalid_argument};

  if (__fc_to_lower(__ptr[0]) != 'a' || __fc_to_lower(__ptr[1]) != 'n')
    return {_Fp{0}, 0, errc::invalid_argument};

  __ptr += 2;

  // The nan sequence, if any, is consumed only when it is well formed.
  if (__last - __ptr >= 2 && __ptr[0] == '(') {
    ptrdiff_t __offset = 1;
    do {
      if (__ptr[__offset] == ')') {
        __ptr += __offset + 1;
        break;
      }
      if (__ptr[__offset] != '_' && !__fc_is_alnum(__ptr[__offset]))
        break;
      ++__offset;
    } while (__ptr + __offset != __last);
  }

  const _Fp __value = numeric_limits<_Fp>::quiet_NaN();
  return {__negative ? -__value : __value, __ptr - __first, errc{}};
}

template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __from_chars_result<_Fp>
__fc_from_chars_hex(const char* __first, const char* __last, const char* __ptr, bool __negative) {
  using __t       = __fc_traits<_Fp>;
  const ptrdiff_t __n = __last - __first;

  const __fc_fractional __fractional = __fc_parse_hex_fractional(__first, __n, __ptr - __first);
  if (!__fractional.__valid)
    return {_Fp{0}, 0, errc::invalid_argument};

  const __fc_exponent __parsed = __fc_parse_exponent(__first, __n, __fractional.__offset, 'p');
  const int __exponent = __fc_merge_exponents(__fractional.__exponent, __parsed.__value, __t::__max_bin, __t::__min_bin);

  __from_chars_result<_Fp> __result{_Fp{0}, __parsed.__offset, {}};
  if (!__fractional.__mantissa.__is_zero()) {
    __fc_big_int __num = __fractional.__mantissa;
    __fc_big_int __den;
    __den.__set(1);
    if (__exponent >= 0)
      __num.__shift_left(__exponent);
    else
      __den.__shift_left(-__exponent);

    bool __oor      = false;
    __result.__value = __fc_round_ratio<_Fp>(__num, __den, __fractional.__truncated, __negative, __oor);
    if (__oor)
      __result.__ec = errc::result_out_of_range;
  } else {
    __result.__value = __fc_from_bits<_Fp>(__negative ? (uint64_t(1) << (__t::__ebits + __t::__p - 1)) : 0);
  }

  return __result;
}

template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __from_chars_result<_Fp> __fc_from_chars_decimal(
    const char* __first, const char* __last, chars_format __fmt, const char* __ptr, bool __negative) {
  using __t           = __fc_traits<_Fp>;
  const ptrdiff_t __n = __last - __first;

  const __fc_fractional __fractional = __fc_parse_decimal_fractional(__first, __n, __ptr - __first);
  if (!__fractional.__valid)
    return {_Fp{0}, 0, errc::invalid_argument};

  ptrdiff_t __offset = __fractional.__offset;
  int __exponent;
  if (__fmt == chars_format::fixed) {
    // LWG3456: a possible exponent is not part of a fixed value; it ends the
    // number instead of making it invalid.
    __exponent = __fc_merge_exponents(__fractional.__exponent, 0, __t::__max_dec, __t::__min_dec);
  } else {
    const __fc_exponent __parsed = __fc_parse_exponent(__first, __n, __offset, 'e');
    if (__fmt == chars_format::scientific && !__parsed.__present) {
      // [charconv.from.chars]/6.2: the exponent is mandatory for scientific.
      return {_Fp{0}, 0, errc::invalid_argument};
    }
    __offset   = __parsed.__offset;
    __exponent = __fc_merge_exponents(__fractional.__exponent, __parsed.__value, __t::__max_dec, __t::__min_dec);
  }

  __from_chars_result<_Fp> __result{_Fp{0}, __offset, {}};
  if (!__fractional.__mantissa.__is_zero()) {
    __fc_big_int __num = __fractional.__mantissa;
    __fc_big_int __den;
    __den.__set(1);
    if (__exponent >= 0)
      __num.__mul_pow10(__exponent);
    else
      __den.__mul_pow10(-__exponent);

    bool __oor        = false;
    __result.__value  = __fc_round_ratio<_Fp>(__num, __den, __fractional.__truncated, __negative, __oor);
    if (__oor)
      __result.__ec = errc::result_out_of_range;
  } else {
    __result.__value = __fc_from_bits<_Fp>(__negative ? (uint64_t(1) << (__t::__ebits + __t::__p - 1)) : 0);
  }

  return __result;
}

// The constant evaluation counterpart of __from_chars_floating_point_impl.
template <class _Fp>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline __from_chars_result<_Fp>
__from_chars_floating_point_cx(const char* __first, const char* __last, chars_format __fmt) {
  if (__first == __last)
    return {_Fp{0}, 0, errc::invalid_argument};

  const char* __ptr = __first;
  const bool __negative = *__ptr == '-';
  if (__negative) {
    ++__ptr;
    if (__ptr == __last)
      return {_Fp{0}, 0, errc::invalid_argument};
  }

  switch (__fc_to_lower(*__ptr)) {
  case 'i':
    return __fc_from_chars_inf<_Fp>(__first, __last, __ptr + 1, __negative);
  case 'n':
    if (numeric_limits<_Fp>::has_quiet_NaN)
      return __fc_from_chars_nan<_Fp>(__first, __last, __ptr + 1, __negative);
    return {_Fp{0}, 0, errc::invalid_argument};
  default:
    break;
  }

  if (__fmt == chars_format::hex)
    return __fc_from_chars_hex<_Fp>(__first, __last, __ptr, __negative);

  return __fc_from_chars_decimal<_Fp>(__first, __last, __fmt, __ptr, __negative);
}

#endif // _LIBCPP_STD_VER >= 17

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_IMPL_H

// clang-format on
