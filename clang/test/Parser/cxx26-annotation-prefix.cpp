// RUN: %clang_cc1 -std=c++26 -freflection -fsyntax-only -verify %s
// RUN: %clang_cc1 -std=c++26 -freflection -Werror=attributes -fsyntax-only -verify %s
// RUN: %clang_cc1 -std=c++26 -freflection -ast-dump %s | FileCheck %s
// expected-no-diagnostics

// A C++26 annotation written in *prefix* position (an
// attribute-specifier-seq at the beginning of a simple-declaration) behaves
// exactly like the standard [[=expr]] spelling. When the declaration has no
// declarators and its decl-specifier-seq declares a tag, the annotation
// appertains to the tag; otherwise it appertains to the declarators.
//
// This covers all four parser paths that handle a prefix attribute list:
// ParseDeclOrFunctionDefInternal (file scope), ParseSimpleDeclaration
// (block scope), ParseDeclarationAfterTemplate (template prefix) and the
// ordinary declarator path. In block scope the same ParsedAttributes object
// doubles as the statement attribute list, so the annotations must be
// consumed rather than applied to the enclosing statement a second time.

struct Foo {
  int x = 0;
  constexpr Foo() = default;
  constexpr Foo(int v) : x(v) {}
};
struct Pair {
  int a, b;
  constexpr Pair(int u, int v) : a(u), b(v) {}
};
namespace ns {
struct Widget {
  int n = 0;
  constexpr Widget() = default;
  constexpr Widget(int v) : n(v) {}
};
} // namespace ns

// ---- file scope: free-standing tag declarations ----
[[Foo]] struct P1;
[[Foo()]] struct P2;
[[Foo(7)]] struct P3;
[[Foo{9}]] struct P4;
[[Pair(1, 2)]] struct P5;
[[ns::Widget]] struct P6;
[[::ns::Widget(6)]] struct P7;
[[Foo, Foo(2)]] struct P8;
[[=42]] struct P9;
[[=Foo(3)]] struct P10;
[[Foo]] enum E1 { e1 };
[[Foo]] class P11;
[[Foo]] union P12;
[[Foo]] struct P13 {};

// ---- block scope ----
void local() {
  [[Foo]] struct L1;
  [[Foo(4)]] struct L2 {};
  [[=42]] struct L3;
  [[ns::Widget]] struct L4;
  [[Foo]] struct L5 {};
}

// ---- template prefix ----
template <class T> [[Foo]] struct T1;
template <class T> [[=42]] struct T2;

// ---- class member scope ----
struct Outer {
  [[Foo]] struct Inner;
  [[=42]] struct Inner2;
};

// ---- after the class-key (not a prefix attribute) ----
struct [[Foo]] A1;

// ---- with declarators: prefix annotations appertain to the declarators ----
[[Foo]] struct W1 {} w1;
[[Foo]] int w2;

// ---- mixed prefix lists ----
// Any non-empty prefix specifier appertains to the tag as a whole; ordinary
// attributes in the same `[[]]` are not misplaced.
[[clang::annotate("tag"), Foo]] struct X1;
[[Foo, clang::annotate("tag")]] struct X2;
[[nodiscard]] struct X3;
[[]] struct X4; // empty specifier: no attributes, no diagnostic

// ---- positional attribute as annotation, and combinations ----
struct [[Foo]] C1;
struct [[Foo, clang::annotate("pos")]] C2;
[[Foo]] struct [[Foo(2)]] C3;
[[clang::annotate("pre")]] struct [[Foo(3)]] C4;
struct Outer2 { [[Foo]] struct [[Foo(4)]] Inner2; };

// CHECK: CXXRecordDecl {{.*}} struct P1
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct P8
// CHECK: CXX26AnnotationAttr
// CHECK: CXX26AnnotationAttr
// CHECK: EnumDecl {{.*}} E1
// CHECK: CXX26AnnotationAttr
// CHECK: FunctionDecl {{.*}} local
// CHECK: CXXRecordDecl {{.*}} struct L1
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct L2
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct L3
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct L4
// CHECK: CXX26AnnotationAttr
// CHECK: ClassTemplateDecl {{.*}} T1
// CHECK: CXXRecordDecl {{.*}} struct T1
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct Inner
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct Inner2
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct W1
// CHECK: VarDecl {{.*}} w1
// CHECK: CXX26AnnotationAttr
// CHECK: VarDecl {{.*}} w2 'int'
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct X1
// CHECK: AnnotateAttr
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct X2
// CHECK: CXX26AnnotationAttr
// CHECK: AnnotateAttr
// CHECK: CXXRecordDecl {{.*}} struct X3
// CHECK: WarnUnusedResultAttr
// CHECK: CXXRecordDecl {{.*}} struct C1
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct C2
// CHECK: CXX26AnnotationAttr
// CHECK: AnnotateAttr
// CHECK: CXXRecordDecl {{.*}} struct C3
// CHECK: CXX26AnnotationAttr
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct C4
// CHECK: CXX26AnnotationAttr
// CHECK: AnnotateAttr
// CHECK: CXXRecordDecl {{.*}} struct Inner2
// CHECK: CXX26AnnotationAttr
