// RUN: %clang_cc1 -fsyntax-only -verify %s
// RUN: %clang_cc1 -ast-dump %s | FileCheck %s
// expected-no-diagnostics

// Prefix attribute-specifier-seqs appertain to the declared tag. Normal use
// emits neither "misplaced attributes" nor a fix-it hint, so the old
// fix-it checks are gone. Verify the attribute actually lands on the tag.

[[nodiscard]] enum class E1 { };
[[nodiscard]] enum struct E2 { };
[[nodiscard]] enum          class E3 { };
[[nodiscard]] enum  /*comment*/ class E4 { };
[[nodiscard]] enum { A = 0 };

namespace NS {
  enum class E5;
}

[[nodiscard]] enum class NS::E5 { };

// CHECK: EnumDecl {{.*}} E1
// CHECK: WarnUnusedResultAttr
// CHECK: EnumDecl {{.*}} E2
// CHECK: WarnUnusedResultAttr
// CHECK: EnumDecl {{.*}} E3
// CHECK: WarnUnusedResultAttr
// CHECK: EnumDecl {{.*}} E5
// CHECK: WarnUnusedResultAttr
