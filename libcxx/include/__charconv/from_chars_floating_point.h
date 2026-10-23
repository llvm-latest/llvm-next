// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_H
#define _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_H

#include <__assert>
#include <__charconv/chars_format.h>
#include <__charconv/from_chars_result.h>
#include <__config>
#include <__cstddef/ptrdiff_t.h>
#include <__system_error/errc.h>
#include <__type_traits/is_constant_evaluated.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCPP_STD_VER >= 17
// This header defines __from_chars_result and the constexpr implementation of
// the floating-point conversion. It has to be included outside namespace std.
#  include <__charconv/from_chars_floating_point_impl.h>
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 17

_LIBCPP_BEGIN_EXPLICIT_ABI_ANNOTATIONS
template <class _Fp>
_LIBCPP_EXPORTED_FROM_ABI __from_chars_result<_Fp> __from_chars_floating_point(
    _LIBCPP_NOESCAPE const char* __first, _LIBCPP_NOESCAPE const char* __last, chars_format __fmt);

extern template __from_chars_result<float> __from_chars_floating_point(
    _LIBCPP_NOESCAPE const char* __first, _LIBCPP_NOESCAPE const char* __last, chars_format __fmt);

extern template __from_chars_result<double> __from_chars_floating_point(
    _LIBCPP_NOESCAPE const char* __first, _LIBCPP_NOESCAPE const char* __last, chars_format __fmt);
_LIBCPP_END_EXPLICIT_ABI_ANNOTATIONS

template <class _Fp>
inline _LIBCPP_CONSTEXPR_SINCE_CXX26 _LIBCPP_HIDE_FROM_ABI from_chars_result
__from_chars(const char* __first, const char* __last, _Fp& __value, chars_format __fmt) {
  __from_chars_result<_Fp> __r;
  if (std::__libcpp_is_constant_evaluated()) {
    // Use the header-only implementation during constant evaluation; the
    // out-of-line implementation is still used at run time for performance and
    // ABI compatibility.
    __r = std::__from_chars_floating_point_constexpr<_Fp>(__first, __last, __fmt);
  } else {
    __r = std::__from_chars_floating_point<_Fp>(__first, __last, __fmt);
  }
  if (__r.__ec != errc::invalid_argument)
    __value = __r.__value;
  return {__first + __r.__n, __r.__ec};
}

_LIBCPP_AVAILABILITY_FROM_CHARS_FLOATING_POINT _LIBCPP_HIDE_FROM_ABI inline _LIBCPP_CONSTEXPR_SINCE_CXX26
from_chars_result from_chars(const char* __first, const char* __last, float& __value,
                             chars_format __fmt = chars_format::general) {
  return std::__from_chars<float>(__first, __last, __value, __fmt);
}

_LIBCPP_AVAILABILITY_FROM_CHARS_FLOATING_POINT _LIBCPP_HIDE_FROM_ABI inline _LIBCPP_CONSTEXPR_SINCE_CXX26
from_chars_result from_chars(const char* __first, const char* __last, double& __value,
                             chars_format __fmt = chars_format::general) {
  return std::__from_chars<double>(__first, __last, __value, __fmt);
}

#endif // _LIBCPP_STD_VER >= 17

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___CHARCONV_FROM_CHARS_FLOATING_POINT_H
