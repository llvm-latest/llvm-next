//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_SRC_INCLUDE_TO_CHARS_FLOATING_POINT_H
#define _LIBCPP_SRC_INCLUDE_TO_CHARS_FLOATING_POINT_H

// The floating-point to_chars implementation lives in the installed headers so
// that it can be used in a constant expression (P3652R1). This header only
// forwards to it; the runtime from_chars implementation uses the shared
// floating-point traits it provides.

#include <__charconv/to_chars_floating_point_impl.h>

#endif // _LIBCPP_SRC_INCLUDE_TO_CHARS_FLOATING_POINT_H
