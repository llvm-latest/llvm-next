// RUN: %clang_cc1 -std=c++26 -freflection -fsyntax-only -verify %s
// RUN: %clang_cc1 -std=c++26 -freflection -verify -ast-dump %s | FileCheck %s

// Every non-empty prefix attribute-specifier-seq appertains to the declared
// tag, whether it holds ordinary attributes, C++26 annotations, or a mix:
// normal use emits neither "misplaced attributes" nor a fix-it hint. An
// empty `[[]]` is a silent no-op. A failed class-name construction reports
// only its real construction error.

struct Foo {
  int x = 0;
  constexpr Foo() = default;
  constexpr Foo(int v) : x(v) {}
};
struct Pair { // #PAIR
  int a, b;
  constexpr Pair(int u, int v) : a(u), b(v) {} // #PAIR_CTOR
};

// File scope: mixed prefix lists, then a pure ordinary attribute and an
// empty specifier. All are accepted.
[[clang::annotate("tag"), Foo]] struct M1;
[[Foo, clang::annotate("tag")]] struct M2;
[[clang::annotate("tag")]] struct M3;
[[]] struct M4; // empty specifier: no attributes, no diagnostic

void local() {
  // Block scope behaves identically; the consumed attributes are not
  // re-applied to the enclosing statement.
  [[clang::annotate("tag"), Foo]] struct L1;
  [[Foo]] struct L2;
  [[=42]] struct L3;
  [[clang::annotate("tag")]] struct L4;
}

// A name that is not a type makes the annotation probe fail. The probe must
// restore the original diagnostic severity so the ordinary attribute path
// still emits its "unknown attribute ignored" warning.
[[not_a_type_name]] void fallback(); // expected-warning {{unknown attribute 'not_a_type_name' ignored}}

// A failed construction must not cascade into a spurious misplaced-attribute
// diagnostic; Sema's own error is the only one we want.
[[Pair(1)]] struct F1; // expected-error {{no matching conversion for functional-style cast from 'int' to 'Pair'}} \
                       // expected-note@#PAIR {{candidate constructor (the implicit copy constructor) not viable}} \
                       // expected-note@#PAIR {{candidate constructor (the implicit move constructor) not viable}} \
                       // expected-note@#PAIR_CTOR {{candidate constructor not viable: requires 2 arguments, but 1 was provided}}

// Prefix attributes (mixed and purely ordinary) appertain to the tag.
// CHECK: CXXRecordDecl {{.*}} struct M1
// CHECK: AnnotateAttr
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct M2
// CHECK: CXX26AnnotationAttr
// CHECK: AnnotateAttr
// CHECK: CXXRecordDecl {{.*}} struct M3
// CHECK: AnnotateAttr
// CHECK-NOT: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct M4
// CHECK-NOT: CXX26AnnotationAttr
// CHECK: FunctionDecl {{.*}} local
// CHECK: CXXRecordDecl {{.*}} struct L1
// CHECK: AnnotateAttr
// CHECK: CXX26AnnotationAttr
// CHECK: CXXRecordDecl {{.*}} struct L4
// CHECK: AnnotateAttr
