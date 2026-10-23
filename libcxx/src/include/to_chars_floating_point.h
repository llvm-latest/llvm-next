//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// The implementation of the floating-point to_chars overloads now lives in the
// public header <__charconv/to_chars_floating_point_impl.h> so that it can be
// used during constant evaluation. This header remains as a compatibility
// shim for the library sources that used to include the implementation here.

#ifndef _LIBCPP_SRC_INCLUDE_TO_CHARS_FLOATING_POINT_H
#define _LIBCPP_SRC_INCLUDE_TO_CHARS_FLOATING_POINT_H

#include <__charconv/to_chars_floating_point_impl.h>

#endif // _LIBCPP_SRC_INCLUDE_TO_CHARS_FLOATING_POINT_H
