//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// XFAIL: availability-fp_from_chars-missing

// <charconv>

// constexpr from_chars_result from_chars(const char* first, const char* last,
//                                        float& value, chars_format fmt = chars_format::general);
// constexpr from_chars_result from_chars(const char* first, const char* last,
//                                        double& value, chars_format fmt = chars_format::general);
// ... and the corresponding overload for long double.

#include <cassert>
#include <charconv>
#include <cstddef>
#include <limits>
#include <system_error>

#include "test_macros.h"

using std::chars_format;
using std::errc;

struct result {
  double value;
  std::ptrdiff_t offset;
  errc ec;
};

template <std::size_t N>
TEST_CONSTEXPR_CXX26 result from_chars_double(const char (&input)[N], chars_format fmt = chars_format::general) {
  double value = 42.0;
  const std::from_chars_result r = std::from_chars(input, input + N - 1, value, fmt);
  return {value, r.ptr - input, r.ec};
}

template <std::size_t N>
TEST_CONSTEXPR_CXX26 result from_chars_float(const char (&input)[N], chars_format fmt = chars_format::general) {
  float value = 42.0f;
  const std::from_chars_result r = std::from_chars(input, input + N - 1, value, fmt);
  return {static_cast<double>(value), r.ptr - input, r.ec};
}

TEST_CONSTEXPR_CXX26 bool test() {
  // Decimal values.
  if (from_chars_double("1.5").value != 1.5 || from_chars_double("1.5").offset != 3 ||
      from_chars_double("1.5").ec != errc{})
    return false;
  if (from_chars_double("-0.5").value != -0.5)
    return false;
  if (from_chars_double("1e10").value != 1e10)
    return false;
  if (from_chars_double("0").value != 0.0 || from_chars_double("0").offset != 1)
    return false;
  if (from_chars_double("-0").value != 0.0 || from_chars_double("-0").offset != 2)
    return false;
  if (from_chars_double("1.7976931348623157e308").value != std::numeric_limits<double>::max())
    return false;
  if (from_chars_double("2.2250738585072014e-308").value != std::numeric_limits<double>::min())
    return false;
  if (from_chars_double("4.9406564584124654e-324").value != std::numeric_limits<double>::denorm_min())
    return false;
  if (from_chars_double("2.2250738585072011e-308").value != 2.2250738585072011e-308) // largest subnormal
    return false;

  // Round to nearest, ties to even.
  if (from_chars_double("9007199254740993").value != 9007199254740992.0)
    return false;
  if (from_chars_double("9007199254740995").value != 9007199254740996.0)
    return false;

  // A long significand is rounded correctly, including values whose digits
  // do not fit in a 64-bit integer.
  if (from_chars_double("32391984281487775748").value != 32391984281487775748.0)
    return false;
  if (from_chars_double("48199.897068901791169e-232").value != 48199.897068901791169e-232)
    return false;
  if (from_chars_double("1.0000000000000000000000000000000000000001").value != 1.0)
    return false;
  if (from_chars_double("0.10000000000000000000000000000000000000001").value != 0.1)
    return false;

  // Formats.
  if (from_chars_double("1.5e3", chars_format::general).value != 1500.0)
    return false;
  if (from_chars_double("1.5e3", chars_format::scientific).value != 1500.0)
    return false;
  if (from_chars_double("1.5e3", chars_format::fixed).value != 1.5 ||
      from_chars_double("1.5e3", chars_format::fixed).offset != 3)
    return false;
  if (from_chars_double("1.5", chars_format::scientific).ec != errc::invalid_argument)
    return false;

  // Hexadecimal values. The 0x prefix is not part of the pattern.
  if (from_chars_double("1.8p1", chars_format::hex).value != 3.0)
    return false;
  if (from_chars_double("-.5p1", chars_format::hex).value != -0.625)
    return false;
  if (from_chars_double("ff", chars_format::hex).value != 255.0)
    return false;
  if (from_chars_double("1p3", chars_format::hex).value != 8.0)
    return false;
  if (from_chars_double("0.8", chars_format::hex).value != 0.5)
    return false;
  if (from_chars_double("0x1p3", chars_format::hex).offset != 1) // 'x' ends the number
    return false;

  // Special values.
  if (from_chars_double("inf").value != std::numeric_limits<double>::infinity() ||
      from_chars_double("inf").offset != 3)
    return false;
  if (from_chars_double("-infinity").value != -std::numeric_limits<double>::infinity() ||
      from_chars_double("-infinity").offset != 9)
    return false;
  if (from_chars_double("INFINITZ").offset != 3)
    return false;
  if (from_chars_double("nan").value == from_chars_double("nan").value)
    return false;
  if (from_chars_double("nan(abc_1)").offset != 10)
    return false;
  if (from_chars_double("-NaN").value == from_chars_double("-NaN").value)
    return false;

  // Errors. When the pattern does not match, value is left unmodified.
  if (from_chars_double("").ec != errc::invalid_argument)
    return false;
  if (from_chars_double("-").ec != errc::invalid_argument)
    return false;
  if (from_chars_double(" 1").ec != errc::invalid_argument)
    return false;
  if (from_chars_double("+1").ec != errc::invalid_argument)
    return false;
  if (from_chars_double("abc").ec != errc::invalid_argument)
    return false;
  if (from_chars_double("abc").value != 42.0)
    return false;
  if (from_chars_double("1.8e308").ec != errc::result_out_of_range)
    return false;
  if (from_chars_double("1.8e308").value != std::numeric_limits<double>::infinity())
    return false;
  if (from_chars_double("1e-400").ec != errc::result_out_of_range)
    return false;
  if (from_chars_double("1e-400").value != 0.0)
    return false;
  if (from_chars_double("1e999999999").ec != errc::result_out_of_range)
    return false;

  // The exponent is clamped, so inputs with arbitrarily large exponents are
  // handled without overflowing any intermediate state.
  if (from_chars_double("1e999999999999999999999").ec != errc::result_out_of_range)
    return false;
  if (from_chars_double("1e-999999999999999999999").ec != errc::result_out_of_range)
    return false;

  // Partial consumption.
  if (from_chars_double("1.2.3").value != 1.2 || from_chars_double("1.2.3").offset != 3)
    return false;
  if (from_chars_double("12x").value != 12.0 || from_chars_double("12x").offset != 2)
    return false;
  if (from_chars_double("1e").value != 1.0 || from_chars_double("1e").offset != 1)
    return false;
  if (from_chars_double("1e+").value != 1.0 || from_chars_double("1e+").offset != 1)
    return false;
  if (from_chars_double("1e5x").offset != 3)
    return false;

  // float overloads.
  if (from_chars_float("3.4028235e38").value != std::numeric_limits<float>::max())
    return false;
  if (from_chars_float("1.17549435e-38").value != std::numeric_limits<float>::min())
    return false;
  if (from_chars_float("1.4e-45").value != std::numeric_limits<float>::denorm_min())
    return false;
  if (from_chars_float("3.5e38").ec != errc::result_out_of_range)
    return false;
  if (from_chars_float("0.1").value != 0.1f)
    return false;
  if (from_chars_float("1.8p1", chars_format::hex).value != 3.0)
    return false;

  return true;
}

int main(int, char**) {
  static_assert(test(), "");
  assert(test());
  return 0;
}
