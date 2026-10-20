// RUN: %clang_cc1 -fsyntax-only -verify %s
// RUN: not %clang_cc1 -fsyntax-only -fdiagnostics-parseable-fixits -fno-diagnostics-show-line-numbers %s 2>&1 | FileCheck %s -strict-whitespace

// A C++11 attribute-specifier-seq preceding an enum definition is ambiguous
// when the same declaration also declares a variable: the attributes could
// appertain to the enum or to the variable. Check the fix-it that moves the
// attributes to the enum-key, disambiguating towards the enum.

[[nodiscard]] enum class E1 { } e1;
// expected-error@-1 {{ambiguous attribute specifier sequence preceding 'enum'}}
// CHECK: {{^}}{{\[\[}}nodiscard]] enum class E1 { } e1;
// CHECK: {{^}}~~~~~~~~~~~~~           ^
// CHECK: fix-it:"{{.*}}":{[[@LINE-4]]:1-[[@LINE-4]]:15}:""
// CHECK: fix-it:"{{.*}}":{[[@LINE-5]]:25-[[@LINE-5]]:25}:"{{\[\[}}nodiscard]]"

[[nodiscard]] enum struct E2 { } e2;
// expected-error@-1 {{ambiguous attribute specifier sequence preceding 'enum'}}
// CHECK: {{^}}{{\[\[}}nodiscard]] enum struct E2 { } e2;
// CHECK: {{^}}~~~~~~~~~~~~~            ^
// CHECK: fix-it:"{{.*}}":{[[@LINE-4]]:1-[[@LINE-4]]:15}:""
// CHECK: fix-it:"{{.*}}":{[[@LINE-5]]:26-[[@LINE-5]]:26}:"{{\[\[}}nodiscard]]"

[[nodiscard]] enum          class E3 { } e3;
// expected-error@-1 {{ambiguous attribute specifier sequence preceding 'enum'}}
// CHECK: {{^}}{{\[\[}}nodiscard]] enum          class E3 { } e3;
// CHECK: {{^}}~~~~~~~~~~~~~                    ^
// CHECK: fix-it:"{{.*}}":{[[@LINE-4]]:1-[[@LINE-4]]:15}:""
// CHECK: fix-it:"{{.*}}":{[[@LINE-5]]:34-[[@LINE-5]]:34}:"{{\[\[}}nodiscard]]"

[[nodiscard]] enum  /*comment*/ class E4 { } e4;
// expected-error@-1 {{ambiguous attribute specifier sequence preceding 'enum'}}
// CHECK: {{^}}{{\[\[}}nodiscard]] enum  /*comment*/ class E4 { } e4;
// CHECK: {{^}}~~~~~~~~~~~~~                        ^
// CHECK: fix-it:"{{.*}}":{[[@LINE-4]]:1-[[@LINE-4]]:15}:""
// CHECK: fix-it:"{{.*}}":{[[@LINE-5]]:38-[[@LINE-5]]:38}:"{{\[\[}}nodiscard]]"

[[nodiscard]] enum { A = 0 } e5;
// expected-error@-1 {{ambiguous attribute specifier sequence preceding 'enum'}}
// CHECK: {{^}}{{\[\[}}nodiscard]] enum { A = 0 } e5;
// CHECK: {{^}}~~~~~~~~~~~~~     ^
// CHECK: fix-it:"{{.*}}":{[[@LINE-4]]:1-[[@LINE-4]]:15}:""
// CHECK: fix-it:"{{.*}}":{[[@LINE-5]]:19-[[@LINE-5]]:19}:"{{\[\[}}nodiscard]]"

namespace NS {
  enum class E6;
}

[[nodiscard]] enum class NS::E6 { } e6;
// expected-error@-1 {{ambiguous attribute specifier sequence preceding 'enum'}}
// CHECK: {{^}}{{\[\[}}nodiscard]] enum class NS::E6 { } e6;
// CHECK: {{^}}~~~~~~~~~~~~~           ^
// CHECK: fix-it:"{{.*}}":{[[@LINE-4]]:1-[[@LINE-4]]:15}:""
// CHECK: fix-it:"{{.*}}":{[[@LINE-5]]:25-[[@LINE-5]]:25}:"{{\[\[}}nodiscard]]"
