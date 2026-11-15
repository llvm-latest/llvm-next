// RUN: %clang_cc1 -std=c++26 -freflection -fsyntax-only -verify %s

// A class-name annotation is shorthand for the standard [[=expr]] spelling:
// both spellings build the same Expr and both end up in
// handleCXX2CAnnotation(), which evaluates the value as a constant
// expression. A non-constexpr constructor is therefore rejected exactly the
// same way - no parse-time short circuit, no silently accepted annotation,
// no different diagnostic text. The same holds for a value whose type is not
// structural.
//
// Each case uses its own constructor so every "declared here" note has a
// unique source location.

struct D1 { int x; D1() : x(1) {} };  // #D1
struct D2 { int x; D2() : x(1) {} };  // #D2
struct D3 { int x; D3() : x(1) {} };  // #D3
struct D4 { int x; D4(int v) : x(v) {} };  // #D4
struct D5 { int x; D5(int v) : x(v) {} };  // #D5
struct D6 { int x; D6(int v) : x(v) {} };  // #D6
namespace ns {
struct D7 { int x; D7() : x(1) {} };  // #D7
struct D8 { int x; D8() : x(1) {} };  // #D8
} // namespace ns
struct D9 { int x; D9() : x(1) {} };  // #D9
struct D10 { int x; D10() : x(1) {} }; // #D10

// ---- baseline: standard [[=expr]] annotations ----
[[=D1()]] struct B1; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                     // expected-note {{non-constexpr constructor 'D1' cannot be used in a constant expression}} \
                     // expected-note@#D1 {{declared here}}
[[=D4(7)]] struct B4; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                      // expected-note {{non-constexpr constructor 'D4' cannot be used in a constant expression}} \
                      // expected-note@#D4 {{declared here}}

// ---- shorthand forms: identical diagnostics ----
[[D2]] struct S2; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                  // expected-note {{non-constexpr constructor 'D2' cannot be used in a constant expression}} \
                  // expected-note@#D2 {{declared here}}
[[D3()]] struct S3; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                    // expected-note {{non-constexpr constructor 'D3' cannot be used in a constant expression}} \
                    // expected-note@#D3 {{declared here}}
[[D5(7)]] struct S5; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                     // expected-note {{non-constexpr constructor 'D5' cannot be used in a constant expression}} \
                     // expected-note@#D5 {{declared here}}
[[D6{7}]] struct S6; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                     // expected-note {{non-constexpr constructor 'D6' cannot be used in a constant expression}} \
                     // expected-note@#D6 {{declared here}}
[[ns::D7]] struct S7; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                      // expected-note {{non-constexpr constructor 'D7' cannot be used in a constant expression}} \
                      // expected-note@#D7 {{declared here}}
[[::ns::D8]] struct S8; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                        // expected-note {{non-constexpr constructor 'D8' cannot be used in a constant expression}} \
                        // expected-note@#D8 {{declared here}}

// Prefix annotations that appertain to a declarator, and ones in block
// scope, are handled by the very same Sema path.
[[D9]] int value; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                  // expected-note {{non-constexpr constructor 'D9' cannot be used in a constant expression}} \
                  // expected-note@#D9 {{declared here}}
void local() {
  [[D10]] struct Local; // expected-error {{C++26 annotation attribute requires an expression usable as a template argument}} \
                        // expected-note {{non-constexpr constructor 'D10' cannot be used in a constant expression}} \
                        // expected-note@#D10 {{declared here}}
}

// ---- non-structural value type ----
struct NS1 { constexpr NS1() = default; private: int x = 0; };
struct NS2 { constexpr NS2() = default; private: int x = 0; };
[[=NS1()]] struct NB1; // expected-error {{C++26 annotation attribute requires a value of structural type}}
[[NS2]] struct NB2; // expected-error {{C++26 annotation attribute requires a value of structural type}}

// ---- constexpr constructors remain valid in every shorthand form ----
struct Ok {
  int x = 0;
  constexpr Ok() = default;
  constexpr Ok(int v) : x(v) {}
};
[[Ok]] struct V1;
[[Ok()]] struct V2;
[[Ok(7)]] struct V3;
[[Ok{9}]] struct V4;
