// RUN: %clang_cc1 -std=c++26 -fsyntax-only -verify %s

// Companion to cxx26-annotation-class-name.cpp: the class-name annotation
// shorthand is experimental and only enabled by -freflection (P3394).
// Without it these are ordinary (unknown) attributes; prefix specifiers
// appertain to the tag, so only the usual unknown-attribute warning appears.
// Also guards against crashes/stray tokens of the earlier speculative impl.

struct Foo {
  int x = 0;
  constexpr Foo() = default;
  constexpr Foo(int v) : x(v) {}
};
struct Bar {
  int a, b;
  constexpr Bar(int u, int v) : a(u), b(v) {}
};
namespace ns { struct Widget {}; }

// Without -freflection the class-name shorthand is an ordinary unknown
// attribute: as a prefix specifier it appertains to the tag, so only the
// "unknown attribute ignored" warning appears - no misplaced error or fix-it.
[[Foo]] struct S1; // expected-warning {{unknown attribute 'Foo' ignored}}
[[Foo(7)]] struct S2; // expected-warning {{unknown attribute 'Foo' ignored}}
[[Bar(1, 2)]] struct S3; // expected-warning {{unknown attribute 'Bar' ignored}}
[[ns::Widget]] struct S4; // expected-warning {{unknown attribute 'ns::Widget' ignored}}

// The same prefix position inside a block behaves identically: the unknown
// attribute is applied to the tag, with no "misplaced" or fix-it diagnostic.
void local() {
  [[Foo]] struct L1; // expected-warning {{unknown attribute 'Foo' ignored}}
  [[Foo(7)]] struct L2; // expected-warning {{unknown attribute 'Foo' ignored}}
}

// Ordinary scoped attributes keep working (these used to emit a spurious
// "undeclared identifier 'clang'" from the annotation probe).
struct Box {
  int v;
  int &get() [[clang::lifetimebound]] { return v; }
};
int &first([[clang::lifetimebound]] Box &&b) { return b.v; }

// Keyword attribute names must not be treated as class names (the earlier
// speculative-reparse implementation asserted here).
[[int(), noreturn]]; // expected-warning {{unknown attribute 'int' ignored}} \
                     // expected-error {{'noreturn' attribute only applies to functions}}

// An unknown identifier that is not a type stays an unknown attribute.
[[some_unknown_name]] void g(); // expected-warning {{unknown attribute 'some_unknown_name' ignored}}
