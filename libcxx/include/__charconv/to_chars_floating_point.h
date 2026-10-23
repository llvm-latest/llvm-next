// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___CHARCONV_TO_CHARS_FLOATING_POINT_H
#define _LIBCPP___CHARCONV_TO_CHARS_FLOATING_POINT_H

#include <__charconv/chars_format.h>
#include <__charconv/to_chars_result.h>
#include <__config>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCPP_STD_VER >= 17
// This header defines _Floating_to_chars and the Ryu conversion routines. It
// has to be included outside namespace std.
#  include <__charconv/to_chars_floating_point_impl.h>
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 17

_LIBCPP_BEGIN_EXPLICIT_ABI_ANNOTATIONS

// Plain overloads.
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, float __value) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Plain>(__first, __last, __value, chars_format{}, 0);
}

inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, double __value) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Plain>(__first, __last, __value, chars_format{}, 0);
}

inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, long double __value) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Plain>(
      __first, __last, static_cast<double>(__value), chars_format{}, 0);
}

// Format-only overloads.
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, float __value, chars_format __fmt) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Format_only>(__first, __last, __value, __fmt, 0);
}

inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, double __value, chars_format __fmt) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Format_only>(__first, __last, __value, __fmt, 0);
}

inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, long double __value, chars_format __fmt) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Format_only>(
      __first, __last, static_cast<double>(__value), __fmt, 0);
}

// Format + precision overloads.
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, float __value, chars_format __fmt, int __precision) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Format_precision>(
      __first, __last, __value, __fmt, __precision);
}

inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, double __value, chars_format __fmt, int __precision) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Format_precision>(
      __first, __last, __value, __fmt, __precision);
}

inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI to_chars_result
to_chars(char* __first, char* __last, long double __value, chars_format __fmt, int __precision) {
  return _Floating_to_chars<_Floating_to_chars_overload::_Format_precision>(
      __first, __last, static_cast<double>(__value), __fmt, __precision);
}

_LIBCPP_END_EXPLICIT_ABI_ANNOTATIONS

#endif // _LIBCPP_STD_VER >= 17

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___CHARCONV_TO_CHARS_FLOATING_POINT_H
