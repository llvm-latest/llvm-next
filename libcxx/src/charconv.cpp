//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <charconv>
#include <string.h>

#include "include/from_chars_floating_point.h"
#include "include/to_chars_floating_point.h"

_LIBCPP_BEGIN_NAMESPACE_STD
_LIBCPP_BEGIN_EXPLICIT_ABI_ANNOTATIONS

#if _LIBCPP_AVAILABILITY_MINIMUM_HEADER_VERSION < 15

namespace __itoa {

_LIBCPP_DIAGNOSTIC_PUSH
_LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wmissing-prototypes")
// These functions exist for ABI compatibility, so we don't ever want a declaration prior to the definition.
_LIBCPP_EXPORTED_FROM_ABI char* __u32toa(uint32_t value, char* buffer) noexcept { return __base_10_u32(buffer, value); }
_LIBCPP_EXPORTED_FROM_ABI char* __u64toa(uint64_t value, char* buffer) noexcept { return __base_10_u64(buffer, value); }
_LIBCPP_DIAGNOSTIC_POP

} // namespace __itoa

#endif // _LIBCPP_AVAILABILITY_MINIMUM_HEADER_VERSION < 15

// The to_chars overloads are now defined inline constexpr in the public header
// <__charconv/to_chars_floating_point.h>, so no out-of-line definitions are
// needed here. The exported __from_chars_floating_point entry points are kept
// for ABI compatibility and are still used at run time, while the constant
// evaluation path uses the header-only implementation.

template <class _Fp>
__from_chars_result<_Fp> __from_chars_floating_point(
    _LIBCPP_NOESCAPE const char* __first, _LIBCPP_NOESCAPE const char* __last, chars_format __fmt) {
  return std::__from_chars_floating_point_impl<_Fp>(__first, __last, __fmt);
}

template __from_chars_result<float> __from_chars_floating_point(
    _LIBCPP_NOESCAPE const char* __first, _LIBCPP_NOESCAPE const char* __last, chars_format __fmt);

template __from_chars_result<double> __from_chars_floating_point(
    _LIBCPP_NOESCAPE const char* __first, _LIBCPP_NOESCAPE const char* __last, chars_format __fmt);

_LIBCPP_END_EXPLICIT_ABI_ANNOTATIONS
_LIBCPP_END_NAMESPACE_STD
