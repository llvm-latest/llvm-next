// -*- C++ -*-
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

#ifndef _LIBCPP___CHARCONV_RYU_RYU_H
#define _LIBCPP___CHARCONV_RYU_RYU_H

// Avoid formatting to keep the changes with the original code minimal.
// clang-format off

#include <__charconv/chars_format.h>
#include <__charconv/to_chars_result.h>
#include <__config>
#include <__system_error/errc.h>
#include <__type_traits/is_same.h>
#include <cstdint>

#include "common.h"
#include "digit_table.h"
#include "d2s_intrinsics.h"
#include "d2s_full_table.h"
#include "d2fixed_full_table.h"
#include "d2fixed.h"
#include "d2s.h"
#include "f2s.h"

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Floating>
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline to_chars_result _Floating_to_chars_ryu(
    char* const _First, char* const _Last, const _Floating _Value, const chars_format _Fmt) noexcept {
    if constexpr (is_same<_Floating, float>::value) {
        return __f2s_buffered_n(_First, _Last, _Value, _Fmt);
    } else {
        return __d2s_buffered_n(_First, _Last, _Value, _Fmt);
    }
}

template <class _Floating>
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline to_chars_result _Floating_to_chars_scientific_precision(
    char* const _First, char* const _Last, const _Floating _Value, int _Precision) noexcept {

    // C11 7.21.6.1 "The fprintf function"/5:
    // "A negative precision argument is taken as if the precision were omitted."
    // /8: "e,E [...] if the precision is missing, it is taken as 6"

    if (_Precision < 0) {
        _Precision = 6;
    } else if (_Precision < 1'000'000'000) { // Match ' to fix compilation with GCC in C++11 mode
        // _Precision is ok.
    } else {
        // Avoid integer overflow.
        // (This defensive check is slightly nonconformant; it can be carefully improved in the future.)
        return {_Last, errc::value_too_large};
    }

    return __d2exp_buffered_n(_First, _Last, _Value, static_cast<uint32_t>(_Precision));
}

template <class _Floating>
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX26 inline to_chars_result _Floating_to_chars_fixed_precision(
    char* const _First, char* const _Last, const _Floating _Value, int _Precision) noexcept {

    // C11 7.21.6.1 "The fprintf function"/5:
    // "A negative precision argument is taken as if the precision were omitted."
    // /8: "f,F [...] If the precision is missing, it is taken as 6"

    if (_Precision < 0) {
        _Precision = 6;
    } else if (_Precision < 1'000'000'000) { // Match ' to fix compilation with GCC in C++11 mode
        // _Precision is ok.
    } else {
        // Avoid integer overflow.
        // (This defensive check is slightly nonconformant; it can be carefully improved in the future.)
        return {_Last, errc::value_too_large};
    }

    return __d2fixed_buffered_n(_First, _Last, _Value, static_cast<uint32_t>(_Precision));
}

_LIBCPP_END_NAMESPACE_STD

// clang-format on

#endif // _LIBCPP___CHARCONV_RYU_RYU_H
