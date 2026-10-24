//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_IMPL_H
#define _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_IMPL_H

#include <__bit/bit_cast.h>
#include <__charconv/chars_format.h>
#include <__config>
#include <__cstddef/ptrdiff_t.h>
#include <__cstddef/size_t.h>
#include <__system_error/errc.h>
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

//===----------------------------------------------------------------------===//
// ASCII-only character classification. from_chars is locale independent and the
// replacements below are usable during constant evaluation.
//===----------------------------------------------------------------------===//

inline constexpr char __fc_tolower(char __c) noexcept {
  return (__c >= 'A' && __c <= 'Z') ? static_cast<char>(__c + ('a' - 'A')) : __c;
}

inline constexpr bool __fc_isdigit(char __c) noexcept { return __c >= '0' && __c <= '9'; }

inline constexpr bool __fc_isxdigit(char __c) noexcept {
  return __fc_isdigit(__c) || (__c >= 'a' && __c <= 'f') || (__c >= 'A' && __c <= 'F');
}

inline constexpr bool __fc_isalnum(char __c) noexcept {
  return __fc_isdigit(__c) || (__c >= 'a' && __c <= 'z') || (__c >= 'A' && __c <= 'Z');
}

inline constexpr bool __fc_isspace(char __c) noexcept {
  return __c == ' ' || __c == '\f' || __c == '\n' || __c == '\r' || __c == '\t' || __c == '\v';
}

//===----------------------------------------------------------------------===//
// Floating-point properties needed by the conversion routines.
//===----------------------------------------------------------------------===//

template <class _Fp>
struct __from_chars_float_traits;

template <>
struct __from_chars_float_traits<float> {
  using _Uint_type = uint32_t;

  static constexpr int __digits        = 24;
  static constexpr int __max_exponent  = 128;
  static constexpr int __min_exponent  = -125;
  // Clamping bound for parsed decimal exponents. Any exponent beyond this range
  // results in overflow or underflow for every possible mantissa. The mantissa
  // can be as large as ~1.8e19 (fits in uint64_t), so the bound must account for
  // that: for underflow we need 1.8e19 * 10^exp < FLT_TRUE_MIN (~1.4e-45),
  // giving exp < -64; for overflow 10^exp > FLT_MAX (~3.4e38), giving exp > 38.
  static constexpr int __decimal_clamp = 80;
  // Bound used for the binary exponent of hexadecimal values.
  static constexpr int __max_biased_exponent = 255;
};

template <>
struct __from_chars_float_traits<double> {
  using _Uint_type = uint64_t;

  static constexpr int __digits        = 53;
  static constexpr int __max_exponent  = 1024;
  static constexpr int __min_exponent  = -1021;
  static constexpr int __decimal_clamp = 400;
  static constexpr int __max_biased_exponent = 2047;
};

//===----------------------------------------------------------------------===//
// A small fixed-capacity unsigned big integer. The capacity is chosen so that
// every intermediate value needed to convert a decimal or hexadecimal string to
// a `float` or `double` fits: 10^400 only needs 1329 bits, and the quotient
// normalization adds at most 57 more bits.
//===----------------------------------------------------------------------===//

struct __fc_bignum {
  static constexpr size_t __capacity = 64; // 2048 bits

  uint32_t __limbs[__capacity]{};
  size_t __size = 0; // number of significant limbs, 0 denotes the value zero

  constexpr void __clear() { __size = 0; }

  constexpr bool __is_zero() const { return __size == 0; }

  constexpr void __set(uint64_t __value) {
    if (__value == 0) {
      __size = 0;
      return;
    }
    __limbs[0] = static_cast<uint32_t>(__value);
    __limbs[1] = static_cast<uint32_t>(__value >> 32);
    __size     = __limbs[1] != 0 ? 2 : 1;
  }

  constexpr void __normalize() {
    while (__size != 0 && __limbs[__size - 1] == 0)
      --__size;
  }

  constexpr void __mul_small(uint32_t __factor) {
    uint64_t __carry = 0;
    for (size_t __i = 0; __i != __size; ++__i) {
      uint64_t __tmp = static_cast<uint64_t>(__limbs[__i]) * __factor + __carry;
      __limbs[__i]   = static_cast<uint32_t>(__tmp);
      __carry        = __tmp >> 32;
    }
    if (__carry != 0)
      __limbs[__size++] = static_cast<uint32_t>(__carry);
  }

  constexpr void __shift_left(size_t __bits) {
    if (__size == 0 || __bits == 0)
      return;

    const size_t __whole = __bits / 32;
    const size_t __part  = __bits % 32;

    // First, shift by whole limbs. Process from high to low so that we never
    // overwrite a limb before it has been read.
    for (size_t __i = __size; __i-- != 0;)
      __limbs[__i + __whole] = __limbs[__i];
    for (size_t __i = 0; __i != __whole; ++__i)
      __limbs[__i] = 0;
    __size += __whole;

    // Then, shift by the remaining part bits within each limb. At this point
    // the data lives in limbs [__whole, __size), and shifting within a limb
    // reads and writes the same limb, so there is no aliasing hazard.
    if (__part != 0) {
      uint32_t __carry = 0;
      for (size_t __i = __whole; __i != __size; ++__i) {
        const uint32_t __tmp   = __limbs[__i];
        __limbs[__i]           = (__tmp << __part) | __carry;
        __carry                = __tmp >> (32 - __part);
      }
      if (__carry != 0)
        __limbs[__size++] = __carry;
    }

    __normalize();
  }

  constexpr void __shift_left_one() {
    uint32_t __carry = 0;
    for (size_t __i = 0; __i != __size; ++__i) {
      const uint32_t __tmp = __limbs[__i];
      __limbs[__i]         = (__tmp << 1) | __carry;
      __carry              = __tmp >> 31;
    }
    if (__carry != 0)
      __limbs[__size++] = __carry;
  }

  constexpr void __or_one() {
    if (__size == 0) {
      __limbs[0] = 1;
      __size     = 1;
    } else {
      __limbs[0] |= 1;
    }
  }

  static constexpr size_t __bit_length32(uint32_t __value) {
    size_t __result = 0;
    while (__value != 0) {
      __value >>= 1;
      ++__result;
    }
    return __result;
  }

  constexpr size_t __bit_length() const {
    if (__size == 0)
      return 0;
    return (__size - 1) * 32 + __bit_length32(__limbs[__size - 1]);
  }

  constexpr bool __bit(size_t __index) const {
    const size_t __limb = __index / 32;
    return __limb < __size && ((__limbs[__limb] >> (__index % 32)) & 1) != 0;
  }

  constexpr uint64_t __low64() const {
    uint64_t __result = 0;
    if (__size > 0)
      __result = __limbs[0];
    if (__size > 1)
      __result |= static_cast<uint64_t>(__limbs[1]) << 32;
    return __result;
  }

  // Returns the 64 bits starting at bit `__lo`.
  constexpr uint64_t __bits_from(size_t __lo) const {
    uint64_t __result = 0;
    for (size_t __i = 0; __i != 64; ++__i) {
      if (__bit(__lo + __i))
        __result |= static_cast<uint64_t>(1) << __i;
    }
    return __result;
  }

  // Whether any bit below `__hi` is set.
  constexpr bool __any_bit_below(size_t __hi) const {
    const size_t __whole = __hi / 32;
    for (size_t __i = 0; __i != __whole && __i < __size; ++__i) {
      if (__limbs[__i] != 0)
        return true;
    }
    if (__whole < __size) {
      const size_t __part = __hi % 32;
      const uint32_t __mask = __part == 0 ? 0 : (static_cast<uint32_t>(1) << __part) - 1;
      if ((__limbs[__whole] & __mask) != 0)
        return true;
    }
    return false;
  }

  constexpr int __compare(const __fc_bignum& __other) const {
    if (__size != __other.__size)
      return __size < __other.__size ? -1 : 1;
    for (size_t __i = __size; __i-- != 0;) {
      if (__limbs[__i] != __other.__limbs[__i])
        return __limbs[__i] < __other.__limbs[__i] ? -1 : 1;
    }
    return 0;
  }

  // Requires *this >= __other.
  constexpr void __subtract(const __fc_bignum& __other) {
    int64_t __borrow = 0;
    for (size_t __i = 0; __i != __size; ++__i) {
      int64_t __tmp = static_cast<int64_t>(__limbs[__i]) -
                      static_cast<int64_t>(__i < __other.__size ? __other.__limbs[__i] : 0) - __borrow;
      if (__tmp < 0) {
        __tmp += static_cast<int64_t>(1) << 32;
        __borrow = 1;
      } else {
        __borrow = 0;
      }
      __limbs[__i] = static_cast<uint32_t>(__tmp);
    }
    __normalize();
  }
};

// Computes floor(__numerator / __denominator) and stores the remainder in
// __remainder. The caller guarantees that the quotient fits in 64 bits.
inline constexpr uint64_t __fc_divide(const __fc_bignum& __numerator, const __fc_bignum& __denominator,
                                      __fc_bignum& __remainder) {
  __remainder.__clear();
  uint64_t __quotient = 0;
  for (size_t __i = __numerator.__bit_length(); __i-- != 0;) {
    __remainder.__shift_left_one();
    if (__numerator.__bit(__i))
      __remainder.__or_one();
    __quotient <<= 1;
    if (__remainder.__compare(__denominator) >= 0) {
      __remainder.__subtract(__denominator);
      __quotient |= 1;
    }
  }
  return __quotient;
}

inline constexpr int32_t __fc_bit_length64(uint64_t __value) {
  int32_t __result = 0;
  while (__value != 0) {
    __value >>= 1;
    ++__result;
  }
  return __result;
}

//===----------------------------------------------------------------------===//
// Rounds a value of the form `__significand * 2^__exp2` (with `__sticky`
// denoting additional non-zero magnitude below the least significant bit of the
// significand) to `_Fp` using round-to-nearest, ties-to-even.
//===----------------------------------------------------------------------===//

template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_round_to_float(uint64_t __significand, int32_t __exp2, bool __sticky, bool __negative, ptrdiff_t __n) {
  using __traits = __from_chars_float_traits<_Fp>;
  using __uint   = typename __traits::_Uint_type;

  constexpr int __digits         = __traits::__digits;
  constexpr int __fraction_bits  = __digits - 1;
  constexpr int __max_unbiased   = __traits::__max_exponent - 1;
  constexpr int __bias           = __max_unbiased;
  constexpr int __min_subnormal  = __traits::__min_exponent - __digits;
  constexpr int __total_bits     = int(sizeof(_Fp) * 8);
  constexpr __uint __sign_mask   = __uint(1) << (__total_bits - 1);

  if (__significand == 0)
    return {__negative ? -_Fp(0) : _Fp(0), __n, errc{}};

  const int32_t __top = __exp2 + __fc_bit_length64(__significand) - 1;

  if (__top > __max_unbiased)
    return {__negative ? -numeric_limits<_Fp>::infinity() : numeric_limits<_Fp>::infinity(),
            __n,
            errc::result_out_of_range};

  int32_t __target_exp = __top - __fraction_bits;
  if (__target_exp < __min_subnormal)
    __target_exp = __min_subnormal;

  const int32_t __drop = __target_exp - __exp2;

  uint64_t __mantissa;
  if (__drop <= 0) {
    __mantissa = __significand << (-__drop);
  } else if (__drop < 64) {
    const uint64_t __quotient = __significand >> __drop;
    const bool __guard        = ((__significand >> (__drop - 1)) & 1) != 0;
    const bool __rest =
        __drop > 1 ? (__significand & ((static_cast<uint64_t>(1) << (__drop - 1)) - 1)) != 0 : false;
    __mantissa = __quotient + ((__guard && (__sticky || __rest || (__quotient & 1) != 0)) ? 1 : 0);
  } else if (__drop == 64) {
    const bool __guard = ((__significand >> 63) & 1) != 0;
    const bool __rest  = (__significand & ((static_cast<uint64_t>(1) << 63) - 1)) != 0;
    __mantissa         = (__guard && (__sticky || __rest)) ? 1 : 0;
  } else {
    __mantissa = 0;
  }

  if (__mantissa == (static_cast<uint64_t>(1) << __digits)) {
    // Rounding carried into the next binade.
    __mantissa >>= 1;
    ++__target_exp;
  }

  if (__mantissa == 0)
    return {__negative ? -_Fp(0) : _Fp(0), __n, errc::result_out_of_range};

  const int32_t __unbiased = __target_exp + __fraction_bits;
  if (__unbiased > __max_unbiased)
    return {__negative ? -numeric_limits<_Fp>::infinity() : numeric_limits<_Fp>::infinity(),
            __n,
            errc::result_out_of_range};

  __uint __bits;
  if (__mantissa < (static_cast<uint64_t>(1) << __fraction_bits)) {
    // Subnormal value: the biased exponent is zero.
    __bits = static_cast<__uint>(__mantissa);
  } else {
    __bits = (static_cast<__uint>(__unbiased + __bias) << __fraction_bits) |
             static_cast<__uint>(__mantissa - (static_cast<uint64_t>(1) << __fraction_bits));
  }

  if (__negative)
    __bits |= __sign_mask;

  return {std::__bit_cast<_Fp>(__bits), __n, errc{}};
}

//===----------------------------------------------------------------------===//
// Helpers to convert a parsed significand to a floating-point value.
//===----------------------------------------------------------------------===//

// Converts `__mantissa * 10^__exp10` to `_Fp`. `__truncated` indicates that the
// decimal significand had additional, non-zero digits that were discarded while
// parsing.
template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_decimal_to_float(uint64_t __mantissa, int64_t __exp10,
                      bool __truncated, bool __negative, ptrdiff_t __n) {
  using __traits = __from_chars_float_traits<_Fp>;

  if (__mantissa == 0)
    return {__negative ? -_Fp(0) : _Fp(0), __n, errc{}};

  constexpr int64_t __clamp = __traits::__decimal_clamp;
  if (__exp10 > __clamp)
    __exp10 = __clamp;
  else if (__exp10 < -__clamp)
    __exp10 = -__clamp;

  __fc_bignum __numerator;
  __numerator.__set(static_cast<uint64_t>(__mantissa));

  if (__exp10 >= 0) {
    for (int64_t __i = 0; __i != __exp10; ++__i)
      __numerator.__mul_small(10);

    const size_t __bits = __numerator.__bit_length();
    if (__bits <= 64)
      return __fc_round_to_float<_Fp>(__numerator.__low64(), 0, __truncated, __negative, __n);

    const size_t __shift = __bits - 64;
    const bool __sticky   = __truncated || __numerator.__any_bit_below(__shift);
    return __fc_round_to_float<_Fp>(__numerator.__bits_from(__shift), static_cast<int32_t>(__shift), __sticky,
                                    __negative, __n);
  }

  __fc_bignum __denominator;
  __denominator.__set(1);
  for (int64_t __i = 0; __i != -__exp10; ++__i)
    __denominator.__mul_small(10);

  // Scale the numerator so that the quotient has 57 significant bits. Together
  // with the remainder this is enough to round correctly.
  const int32_t __shift = static_cast<int32_t>(static_cast<int64_t>(__denominator.__bit_length()) + 57 -
                                               static_cast<int64_t>(__numerator.__bit_length()));
  __fc_bignum __divisor = __denominator;
  if (__shift >= 0)
    __numerator.__shift_left(static_cast<size_t>(__shift));
  else
    __divisor.__shift_left(static_cast<size_t>(-__shift));

  __fc_bignum __remainder;
  const uint64_t __quotient = __fc_divide(__numerator, __divisor, __remainder);
  const bool __sticky       = __truncated || !__remainder.__is_zero();

  return __fc_round_to_float<_Fp>(__quotient, -__shift, __sticky, __negative, __n);
}

// Converts `__mantissa * 2^__exp2` to `_Fp`.
template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_binary_to_float(uint64_t __mantissa, int32_t __exp2,
                     bool __truncated, bool __negative, ptrdiff_t __n) {
  if (__mantissa == 0)
    return {__negative ? -_Fp(0) : _Fp(0), __n, errc{}};

  return __fc_round_to_float<_Fp>(static_cast<uint64_t>(__mantissa), __exp2, __truncated, __negative, __n);
}

//===----------------------------------------------------------------------===//
// Parsers. These follow the structure of the implementation in
// libcxx/src/include/from_chars_floating_point.h so that the observable
// behaviour (including accepted patterns and error codes) is identical.
//===----------------------------------------------------------------------===//

template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_parse_inf(const char* const __first, const char* __last, const char* __ptr, bool __negative) {
  if (__last - __ptr < 2)
    return {_Fp{0}, 0, errc::invalid_argument};

  if (__fc_tolower(__ptr[0]) != 'n' || __fc_tolower(__ptr[1]) != 'f')
    return {_Fp{0}, 0, errc::invalid_argument};

  __ptr += 2;

  // At this point "inf" has been parsed. The optional "inity" suffix is consumed
  // when present; otherwise only "inf" is consumed.
  if (__last - __ptr >= 5 && __fc_tolower(__ptr[0]) == 'i' && __fc_tolower(__ptr[1]) == 'n' &&
      __fc_tolower(__ptr[2]) == 'i' && __fc_tolower(__ptr[3]) == 't' && __fc_tolower(__ptr[4]) == 'y')
    __ptr += 5;

  if (numeric_limits<_Fp>::has_infinity) {
    if (__negative)
      return {-numeric_limits<_Fp>::infinity(), __ptr - __first, errc{}};
    return {numeric_limits<_Fp>::infinity(), __ptr - __first, errc{}};
  }
  return {_Fp{0}, __ptr - __first, errc::result_out_of_range};
}

template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_parse_nan(const char* const __first, const char* __last, const char* __ptr, bool __negative) {
  if (__last - __ptr < 2)
    return {_Fp{0}, 0, errc::invalid_argument};

  if (__fc_tolower(__ptr[0]) != 'a' || __fc_tolower(__ptr[1]) != 'n')
    return {_Fp{0}, 0, errc::invalid_argument};

  __ptr += 2;

  // The optional n-char-sequence in parentheses is consumed when complete.
  if (__last - __ptr >= 2 && __ptr[0] == '(') {
    size_t __offset = 1;
    do {
      if (__ptr[__offset] == ')') {
        __ptr += __offset + 1;
        break;
      }
      if (__ptr[__offset] != '_' && !__fc_isalnum(__ptr[__offset]))
        break;
      ++__offset;
    } while (__ptr + __offset != __last);
  }

  if (__negative)
    return {-numeric_limits<_Fp>::quiet_NaN(), __ptr - __first, errc{}};
  return {numeric_limits<_Fp>::quiet_NaN(), __ptr - __first, errc{}};
}

struct __fc_fractional_constant_result {
  ptrdiff_t __offset{ptrdiff_t(-1)};
  uint64_t __mantissa{0};
  int __exponent{0};
  bool __truncated{false};
  bool __is_valid{false};
};

// Parses the decimal significand of a floating-point value.
template <class _Tp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __fc_fractional_constant_result
__fc_parse_decimal_fractional(const char* __input, ptrdiff_t __n, ptrdiff_t __offset) {
  __fc_fractional_constant_result __result;

  // The mantissa is stored in a uint64_t, so use the full 64-bit range as the
  // truncation threshold. Using _Tp (which is 32 bits for float) here would
  // lose precision and cause incorrect rounding for values with more than
  // ~10 significant decimal digits.
  const uint64_t __threshold = numeric_limits<uint64_t>::max() / 10;
  bool __fraction            = false;
  for (; __offset < __n; ++__offset) {
    if (__fc_isdigit(__input[__offset])) {
      __result.__is_valid = true;

      const uint32_t __digit = static_cast<uint32_t>(__input[__offset] - '0');
      if (__result.__mantissa < __threshold) {
        __result.__mantissa = __result.__mantissa * 10 + __digit;
        if (__fraction)
          --__result.__exponent;
      } else {
        if (__digit > 0)
          __result.__truncated = true;
        if (!__fraction)
          ++__result.__exponent;
      }
    } else if (__input[__offset] == '.') {
      if (__fraction)
        break; // A second decimal point ends the number.
      __fraction = true;
    } else {
      break;
    }
  }

  __result.__offset = __offset;
  return __result;
}

// Parses the hexadecimal significand of a floating-point value.
template <class _Tp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __fc_fractional_constant_result
__fc_parse_hex_fractional(const char* __input, ptrdiff_t __n, ptrdiff_t __offset) {
  __fc_fractional_constant_result __result;

  const uint64_t __threshold = numeric_limits<uint64_t>::max() / 16;
  bool __fraction            = false;
  for (; __offset < __n; ++__offset) {
    if (__fc_isxdigit(__input[__offset])) {
      __result.__is_valid = true;

      uint32_t __digit = static_cast<uint32_t>(__input[__offset] - '0');
      switch (__fc_tolower(__input[__offset])) {
      case 'a':
        __digit = 10;
        break;
      case 'b':
        __digit = 11;
        break;
      case 'c':
        __digit = 12;
        break;
      case 'd':
        __digit = 13;
        break;
      case 'e':
        __digit = 14;
        break;
      case 'f':
        __digit = 15;
        break;
      default:
        break;
      }

      if (__result.__mantissa < __threshold) {
        __result.__mantissa = __result.__mantissa * 16 + __digit;
        if (__fraction)
          __result.__exponent -= 4;
      } else {
        if (__digit > 0)
          __result.__truncated = true;
        if (!__fraction)
          __result.__exponent += 4;
      }
    } else if (__input[__offset] == '.') {
      if (__fraction)
        break; // A second decimal point ends the number.
      __fraction = true;
    } else {
      break;
    }
  }

  __result.__offset = __offset;
  return __result;
}

struct __fc_exponent_result {
  ptrdiff_t __offset{ptrdiff_t(-1)};
  int32_t __value{0};
  bool __present{false};
};

// Parses the exponent part of a floating-point value. When no exponent is
// present `__offset` points at the exponent marker (if any).
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __fc_exponent_result
__fc_parse_exponent(const char* __input, ptrdiff_t __n, ptrdiff_t __offset, char __marker) {
  // An exponent always needs at least one digit and leading whitespace is not
  // allowed.
  if (__offset + 1 < __n && __fc_tolower(__input[__offset]) == __marker && !__fc_isspace(__input[__offset + 1])) {
    ptrdiff_t __ptr = __offset + 1;

    bool __negative = false;
    if (__ptr < __n && (__input[__ptr] == '+' || __input[__ptr] == '-')) {
      __negative = __input[__ptr] == '-';
      ++__ptr;
    }

    const ptrdiff_t __digits_first = __ptr;
    int64_t __value                = 0;
    for (; __ptr < __n && __fc_isdigit(__input[__ptr]); ++__ptr) {
      if (__value <= int64_t(numeric_limits<int32_t>::max()))
        __value = __value * 10 + (__input[__ptr] - '0');
      if (__value > int64_t(numeric_limits<int32_t>::max()))
        __value = int64_t(numeric_limits<int32_t>::max());
    }

    if (__ptr == __digits_first)
      return {__offset, 0, false};

    if (__negative)
      __value = -__value;
    return {__ptr, static_cast<int32_t>(__value), true};
  }

  return {__offset, 0, false};
}

// Adds the exponent of the significand and the exponent part. The result is
// clamped, which is harmless: any value outside the clamp range overflows or
// underflows for the type being parsed.
inline constexpr int32_t __fc_merge_exponents(int64_t __fractional, int64_t __exponent, int32_t __clamp) {
  const int64_t __sum = __fractional + __exponent;
  if (__sum > __clamp)
    return __clamp;
  if (__sum < -__clamp)
    return -__clamp;
  return static_cast<int32_t>(__sum);
}

template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_from_chars_hex(const char* const __first, const char* __last, const char* __ptr, bool __negative) {
  using __traits = __from_chars_float_traits<_Fp>;
  using __uint   = typename __traits::_Uint_type;

  const ptrdiff_t __n = __last - __first;
  ptrdiff_t __offset  = __ptr - __first;

  const __fc_fractional_constant_result __fractional = __fc_parse_hex_fractional<__uint>(__first, __n, __offset);
  if (!__fractional.__is_valid)
    return {_Fp{0}, 0, errc::invalid_argument};

  const __fc_exponent_result __parsed_exponent = __fc_parse_exponent(__first, __n, __fractional.__offset, 'p');
  __offset                                     = __parsed_exponent.__offset;

  const int32_t __exponent =
      __fc_merge_exponents(__fractional.__exponent, __parsed_exponent.__value, __traits::__max_biased_exponent);

  return __fc_binary_to_float<_Fp>(__fractional.__mantissa, __exponent,
                                   __fractional.__truncated, __negative, __offset);
}

template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__fc_from_chars_decimal(const char* const __first, const char* __last, chars_format __fmt, const char* __ptr,
                        bool __negative) {
  using __traits = __from_chars_float_traits<_Fp>;
  using __uint   = typename __traits::_Uint_type;

  const ptrdiff_t __n = __last - __first;
  ptrdiff_t __offset  = __ptr - __first;

  const __fc_fractional_constant_result __fractional = __fc_parse_decimal_fractional<__uint>(__first, __n, __offset);
  if (!__fractional.__is_valid)
    return {_Fp{0}, 0, errc::invalid_argument};

  __offset = __fractional.__offset;

  // LWG3456: `fixed` ignores a possible exponent instead of treating its
  // presence as an error.
  int32_t __exponent;
  if (__fmt == chars_format::fixed) {
    __exponent = __fc_merge_exponents(__fractional.__exponent, 0, __traits::__decimal_clamp);
  } else {
    const __fc_exponent_result __parsed_exponent = __fc_parse_exponent(__first, __n, __offset, 'e');
    if (__fmt == chars_format::scientific && !__parsed_exponent.__present)
      return {_Fp{0}, 0, errc::invalid_argument};

    __offset   = __parsed_exponent.__offset;
    __exponent = __fc_merge_exponents(__fractional.__exponent, __parsed_exponent.__value, __traits::__decimal_clamp);
  }

  return __fc_decimal_to_float<_Fp>(__fractional.__mantissa, __exponent,
                                    __fractional.__truncated, __negative, __offset);
}

// Entry point used during constant evaluation.
template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI __from_chars_result<_Fp>
__from_chars_floating_point_constexpr(const char* const __first, const char* __last, chars_format __fmt) {
  if (__first == __last)
    return {_Fp{0}, 0, errc::invalid_argument};

  const char* __ptr    = __first;
  const bool __negative = *__ptr == '-';
  if (__negative) {
    ++__ptr;
    if (__ptr == __last)
      return {_Fp{0}, 0, errc::invalid_argument};
  }

  switch (__fc_tolower(*__ptr)) {
  case 'i':
    return __fc_parse_inf<_Fp>(__first, __last, __ptr + 1, __negative);
  case 'n':
    return __fc_parse_nan<_Fp>(__first, __last, __ptr + 1, __negative);
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
