//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// <charconv>

// constexpr to_chars_result to_chars(char* first, char* last,
//                                    floating-point-type value);
// constexpr to_chars_result to_chars(char* first, char* last,
//                                    floating-point-type value, chars_format fmt);
// constexpr to_chars_result to_chars(char* first, char* last,
//                                    floating-point-type value, chars_format fmt,
//                                    int precision);

#include <bit>
#include <charconv>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <system_error>

#include "test_macros.h"

template <class F>
TEST_CONSTEXPR_CXX26 bool check(std::string_view expected, std::chars_format fmt, F value) {
  char buffer[512];
  std::to_chars_result r;
  if (fmt == std::chars_format{})
    r = std::to_chars(buffer, buffer + sizeof(buffer), value);
  else
    r = std::to_chars(buffer, buffer + sizeof(buffer), value, fmt);
  return r.ec == std::errc{} && std::string_view(buffer, r.ptr - buffer) == expected;
}

TEST_CONSTEXPR_CXX26 bool test_to_chars() {
  // Shortest representations.
  assert(check<double>("1.5", std::chars_format{}, 1.5));
  assert(check<double>("0", std::chars_format{}, 0.0));
  assert(check<double>("-0", std::chars_format{}, -0.0));
  assert(check<double>("123", std::chars_format{}, 123.0));

  // Specific formats.
  assert(check<double>("1.5", std::chars_format::general, 1.5));
  assert(check<double>("1e+00", std::chars_format::scientific, 1.0));
  assert(check<double>("1.5e+02", std::chars_format::scientific, 150.0));
  assert(check<double>("1.5", std::chars_format::fixed, 1.5));

  // Hexadecimal.
  assert(check<double>("1p+0", std::chars_format::hex, 1.0));
  assert(check<double>("-1p+0", std::chars_format::hex, -1.0));

  // Infinities and NaN.
  assert(check<double>("inf", std::chars_format{}, std::numeric_limits<double>::infinity()));
  assert(check<double>("-inf", std::chars_format{}, -std::numeric_limits<double>::infinity()));
  assert(check<double>("nan", std::chars_format{}, std::numeric_limits<double>::quiet_NaN()));

  // The same conversion for float.
  assert(check<float>("1.5", std::chars_format{}, 1.5f));
  assert(check<float>("1p+0", std::chars_format::hex, 1.0f));

  // Precision overloads.
  {
    char buffer[128];
    std::to_chars_result r = std::to_chars(buffer, buffer + sizeof(buffer), 1.0, std::chars_format::fixed, 0);
    assert(r.ec == std::errc{});
    assert(std::string_view(buffer, r.ptr - buffer) == "1");
  }
  {
    char buffer[128];
    std::to_chars_result r = std::to_chars(buffer, buffer + sizeof(buffer), 1.234, std::chars_format::fixed, 2);
    assert(r.ec == std::errc{});
    assert(std::string_view(buffer, r.ptr - buffer) == "1.23");
  }
  {
    char buffer[128];
    std::to_chars_result r = std::to_chars(buffer, buffer + sizeof(buffer), 1.0, std::chars_format::scientific, 1);
    assert(r.ec == std::errc{});
    assert(std::string_view(buffer, r.ptr - buffer) == "1.0e+00");
  }
  {
    char buffer[128];
    std::to_chars_result r = std::to_chars(buffer, buffer + sizeof(buffer), 1.0, std::chars_format::hex, 2);
    assert(r.ec == std::errc{});
    assert(std::string_view(buffer, r.ptr - buffer) == "1.00p+0");
  }

  // Buffer too small.
  {
    char buffer[2];
    std::to_chars_result r = std::to_chars(buffer, buffer + sizeof(buffer), 1234.0);
    assert(r.ec == std::errc::value_too_large);
    assert(r.ptr == buffer + sizeof(buffer));
  }

  return true;
}

template <class F>
TEST_CONSTEXPR_CXX26 uint64_t bits_of(F value) {
  if constexpr (sizeof(F) == sizeof(uint32_t))
    return std::bit_cast<uint32_t>(value);
  else
    return std::bit_cast<uint64_t>(value);
}

template <class F>
TEST_CONSTEXPR_CXX26 void test_round_trip() {
  const F values[] = {
      F(0.1),
      F(1.5),
      F(3.141592653589793),
      F(123456789.123),
      -F(0.0),
      std::numeric_limits<F>::max(),
      std::numeric_limits<F>::denorm_min(),
  };

  for (F value : values) {
    char buffer[512];
    std::to_chars_result w = std::to_chars(buffer, buffer + sizeof(buffer), value);
    assert(w.ec == std::errc{});

    F round_tripped           = 0;
    std::from_chars_result r = std::from_chars(buffer, w.ptr, round_tripped);
    assert(r.ec == std::errc{});
    assert(r.ptr == w.ptr);
    assert(bits_of(round_tripped) == bits_of(value));
  }
}

TEST_CONSTEXPR_CXX26 bool test() {
  test_to_chars();
  test_round_trip<float>();
  test_round_trip<double>();
  return true;
}

static_assert(test(), "");

int main(int, char**) { return 0; }
