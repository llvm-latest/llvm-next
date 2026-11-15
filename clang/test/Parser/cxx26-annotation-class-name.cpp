// RUN: %clang_cc1 -std=c++26 -freflection -fsyntax-only -verify %s
// RUN: %clang_cc1 -std=c++26 -freflection -ast-dump %s | FileCheck %s
// expected-no-diagnostics

// A C++26 annotation can be written by naming a class type directly, with
// an optional nested-name-specifier and optional constructor arguments:
//
//   [[Foo]]          -> Foo{}        (implicit default construction)
//   [[Foo()]]        -> Foo()
//   [[Foo(7)]]       -> Foo(7)
//   [[Foo{9}]]       -> Foo{9}
//   [[ns::Foo]]      -> ns::Foo{}
//   [[::ns::Foo(1)]] -> ::ns::Foo(1)
//
// The annotation value is the constructed temporary, so these are all
// shorthand for the standard `[[=Foo(...)]]` spelling. The feature is
// experimental and gated on -freflection (P3394); the companion test
// cxx26-annotation-class-name-disabled.cpp covers the disabled mode.

struct Foo {
  int x = 0;
  constexpr Foo() = default;
  constexpr Foo(int v) : x(v) {}
};
struct Bar {
  int a, b;
  constexpr Bar(int u, int v) : a(u), b(v) {}
};
namespace ns {
struct Widget {
  int n = 0;
  constexpr Widget() = default;
  constexpr Widget(int v) : n(v) {}
};
} // namespace ns

// Bare class name: implicit default construction.
[[Foo]] struct S1;

// Explicit construction, all three spellings.
[[Foo()]] struct S2;
[[Foo(7)]] struct S3;
[[Foo{9}]] struct S4;
[[Bar(1, 2)]] struct S5;
[[Bar{3, 4}]] struct S6;

// Qualified names, including the global scope specifier.
[[ns::Widget]] struct S7;
[[ns::Widget(5)]] struct S8;
[[::ns::Widget]] struct S9;
[[::ns::Widget(6)]] struct S10;

// Several class-name annotations in one attribute-specifier-seq, and mixed
// with a standard `[[=expr]]` annotation.
[[Foo, Bar(1, 2)]] void f();
[[=42, Foo(5)]] struct S11;

// CHECK: CXXRecordDecl {{.*}} struct S1
// CHECK: CXX26AnnotationAttr
// CHECK: CXXTemporaryObjectExpr {{.*}} 'Foo' 'void () noexcept' zeroing
// CHECK: CXXRecordDecl {{.*}} struct S3
// CHECK: CXX26AnnotationAttr
// CHECK: CXXTemporaryObjectExpr {{.*}} 'Foo' 'void (int)'
// CHECK: CXXRecordDecl {{.*}} struct S7
// CHECK: CXX26AnnotationAttr
// CHECK: CXXTemporaryObjectExpr {{.*}} 'ns::Widget' 'void () noexcept' zeroing
// CHECK: CXXRecordDecl {{.*}} struct S10
// CHECK: CXX26AnnotationAttr
// CHECK: CXXFunctionalCastExpr {{.*}} '::ns::Widget'
