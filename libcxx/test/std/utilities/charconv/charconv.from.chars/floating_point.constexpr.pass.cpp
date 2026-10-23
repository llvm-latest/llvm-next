//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// <charconv>

// constexpr from_chars_result from_chars(const char* first, const char* last,
//                                        floating-point-type& value,
//                                        chars_format fmt = chars_format::general);

#include <charconv>
#include <cassert>
#include <limits>
#include <system_error>

#include "test_macros.h"

template <class F>
TEST_CONSTEXPR_CXX26 void test_success() {
  F value = F(42);

  { // plain decimal
    const char s[] = "1.5";
    std::from_chars_result r = std::from_chars(s, s + 3, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 3);
    assert(value == F(1.5));
  }
  { // negative value
    const char s[] = "-0.25";
    std::from_chars_result r = std::from_chars(s, s + 5, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 5);
    assert(value == F(-0.25));
  }
  { // trailing characters are not consumed
    const char s[] = "123abc";
    std::from_chars_result r = std::from_chars(s, s + 6, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 3);
    assert(value == F(123));
  }
  { // scientific
    const char s[] = "1.5e2";
    std::from_chars_result r = std::from_chars(s, s + 5, value, std::chars_format::scientific);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 5);
    assert(value == F(150));
  }
  { // fixed ignores and does not consume the exponent
    const char s[] = "1.5e2";
    std::from_chars_result r = std::from_chars(s, s + 5, value, std::chars_format::fixed);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 3);
    assert(value == F(1.5));
  }
  { // hexadecimal
    const char s[] = "1.8p1";
    std::from_chars_result r = std::from_chars(s, s + 5, value, std::chars_format::hex);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 5);
    assert(value == F(3));
  }
  { // hexadecimal without a leading digit
    const char s[] = "-.5p1";
    std::from_chars_result r = std::from_chars(s, s + 5, value, std::chars_format::hex);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 5);
    assert(value == F(-1));
  }
  { // infinity, case insensitive, with and without the "inity" suffix
    const char s[] = "-INFINITY";
    std::from_chars_result r = std::from_chars(s, s + 9, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 9);
    assert(value == -std::numeric_limits<F>::infinity());
  }
  {
    const char s[] = "inf";
    std::from_chars_result r = std::from_chars(s, s + 3, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 3);
    assert(value == std::numeric_limits<F>::infinity());
  }
  { // nan, with an n-char-sequence
    const char s[] = "NAN(abc)";
    std::from_chars_result r = std::from_chars(s, s + 8, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 8);
    assert(value != value); // NaN
  }
  { // zero
    const char s[] = "-0.0";
    std::from_chars_result r = std::from_chars(s, s + 4, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 4);
    assert(value == F(0));
  }
}

TEST_CONSTEXPR_CXX26 void test_subnormal() {
  { // smallest subnormal float
    const char s[] = "1e-45";
    float value    = 0;
    std::from_chars_result r = std::from_chars(s, s + 5, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 5);
    assert(value == std::numeric_limits<float>::denorm_min());
  }
  { // smallest subnormal double
    const char s[] = "5e-324";
    double value   = 0;
    std::from_chars_result r = std::from_chars(s, s + 6, value);
    assert(r.ec == std::errc{});
    assert(r.ptr == s + 6);
    assert(value == std::numeric_limits<double>::denorm_min());
  }
}

TEST_CONSTEXPR_CXX26 void test_overflow() {
  { // overflow yields infinity and result_out_of_range
    const char s[] = "1e999";
    float value   = 0;
    std::from_chars_result r = std::from_chars(s, s + 5, value);
    assert(r.ec == std::errc::result_out_of_range);
    assert(r.ptr == s + 5);
    assert(value == std::numeric_limits<float>::infinity());
  }
  { // underflow yields zero and result_out_of_range
    const char s[] = "-1e-999";
    float value   = 0;
    std::from_chars_result r = std::from_chars(s, s + 7, value);
    assert(r.ec == std::errc::result_out_of_range);
    assert(r.ptr == s + 7);
    assert(value == 0);
  }
}

template <class F>
TEST_CONSTEXPR_CXX26 void test_failure() {
  F value = F(42);

  { // empty range
    const char s[] = "";
    std::from_chars_result r = std::from_chars(s, s, value);
    assert(r.ec == std::errc::invalid_argument);
    assert(r.ptr == s);
    assert(value == F(42));
  }
  { // sign without digits
    const char s[] = "-";
    std::from_chars_result r = std::from_chars(s, s + 1, value);
    assert(r.ec == std::errc::invalid_argument);
    assert(r.ptr == s);
    assert(value == F(42));
  }
  { // not a number
    const char s[] = "abc";
    std::from_chars_result r = std::from_chars(s, s + 3, value);
    assert(r.ec == std::errc::invalid_argument);
    assert(r.ptr == s);
    assert(value == F(42));
  }
  { // scientific requires an exponent
    const char s[] = "1.5";
    std::from_chars_result r = std::from_chars(s, s + 3, value, std::chars_format::scientific);
    assert(r.ec == std::errc::invalid_argument);
    assert(r.ptr == s);
    assert(value == F(42));
  }
}

TEST_CONSTEXPR_CXX26 bool test() {
  test_success<float>();
  test_success<double>();
  test_subnormal();
  test_overflow();
  test_failure<float>();
  test_failure<double>();
  return true;
}

static_assert(test(), "");

int main(int, char**) { return 0; }
