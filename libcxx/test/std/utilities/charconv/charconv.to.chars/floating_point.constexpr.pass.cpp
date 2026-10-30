//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// <charconv>

// constexpr to_chars_result to_chars(char* first, char* last, float value);
// constexpr to_chars_result to_chars(char* first, char* last, float value, chars_format fmt);
// constexpr to_chars_result to_chars(char* first, char* last, float value, chars_format fmt, int precision);
// ... and the corresponding overloads for double and long double.

#include <cassert>
#include <charconv>
#include <limits>
#include <system_error>

#include "test_macros.h"

using std::chars_format;

TEST_CONSTEXPR_CXX26 bool equal(const char* first, const char* last, const char* expected) {
  for (; first != last && *expected != '\0'; ++first, ++expected) {
    if (*first != *expected)
      return false;
  }
  return first == last && *expected == '\0';
}

template <class Fp>
TEST_CONSTEXPR_CXX26 bool check(Fp value, const char* expected) {
  char buffer[64];
  const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value);
  return result.ec == std::errc{} && equal(buffer, result.ptr, expected);
}

TEST_CONSTEXPR_CXX26 bool check_fmt(double value, const char* expected, chars_format fmt) {
  char buffer[64];
  const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value, fmt);
  return result.ec == std::errc{} && equal(buffer, result.ptr, expected);
}

TEST_CONSTEXPR_CXX26 bool check_fmt_f(float value, const char* expected, chars_format fmt) {
  char buffer[64];
  const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value, fmt);
  return result.ec == std::errc{} && equal(buffer, result.ptr, expected);
}

TEST_CONSTEXPR_CXX26 bool test() {
  // Default formatting.
  if (!check(0.0, "0"))
    return false;
  if (!check(-0.0, "-0"))
    return false;
  if (!check(1.0, "1"))
    return false;
  if (!check(-1.5, "-1.5"))
    return false;
  if (!check(1234.5, "1234.5"))
    return false;
  if (!check(1e300, "1e+300"))
    return false;
  if (!check(0.1, "0.1"))
    return false;
  if (!check(1.0 / 3.0, "0.3333333333333333"))
    return false;
  if (!check(std::numeric_limits<double>::max(), "1.7976931348623157e+308"))
    return false;
  if (!check(std::numeric_limits<double>::min(), "2.2250738585072014e-308"))
    return false;
  if (!check(std::numeric_limits<double>::denorm_min(), "5e-324"))
    return false;

  // float overloads round trip through their own shortest representation.
  if (!check(0.0f, "0"))
    return false;
  if (!check(0.1f, "0.1"))
    return false;
  if (!check(1.0f / 3.0f, "0.33333334"))
    return false;
  if (!check(std::numeric_limits<float>::max(), "3.4028235e+38"))
    return false;
  if (!check(std::numeric_limits<float>::denorm_min(), "1e-45"))
    return false;

  // Explicit formats.
  if (!check_fmt(1.5, "1.5", chars_format::general))
    return false;
  if (!check_fmt(1.5, "1.5", chars_format::fixed))
    return false;
  if (!check_fmt(1.5, "1.5e+00", chars_format::scientific))
    return false;
  if (!check_fmt(255.0, "1.fep+7", chars_format::hex))
    return false;
  if (!check_fmt(0.0, "0", chars_format::fixed))
    return false;
  if (!check_fmt(0.0, "0e+00", chars_format::scientific))
    return false;
  if (!check_fmt_f(1.5f, "1.5", chars_format::general))
    return false;
  if (!check_fmt_f(1.5f, "1.5e+00", chars_format::scientific))
    return false;

  // The buffer is always large enough, so no error can occur.
  char buffer[8];
  const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), 1.0);
  if (result.ec != std::errc{})
    return false;

  return true;
}

int main(int, char**) {
  static_assert(test(), "");
  assert(test());
  return 0;
}
