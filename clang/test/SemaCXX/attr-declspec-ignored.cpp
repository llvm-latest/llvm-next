// RUN: %clang_cc1 %s -verify -fsyntax-only
// RUN: %clang_cc1 %s -std=c++03 -Wno-c++11-extensions -verify -fsyntax-only

namespace test1 {
  __attribute__((visibility("hidden")))  __attribute__((aligned)) class A; // expected-warning{{attribute 'visibility' is ignored, place it after "class" to apply attribute to type declaration}} \
  // expected-warning{{attribute 'aligned' is ignored, place it after "class" to apply attribute to type declaration}}
  __attribute__((visibility("hidden")))  __attribute__((aligned)) struct B; // expected-warning{{attribute 'visibility' is ignored, place it after "struct" to apply attribute to type declaration}} \
  // expected-warning{{attribute 'aligned' is ignored, place it after "struct" to apply attribute to type declaration}}
  __attribute__((visibility("hidden")))  __attribute__((aligned)) union C; // expected-warning{{attribute 'visibility' is ignored, place it after "union" to apply attribute to type declaration}} \
  // expected-warning{{attribute 'aligned' is ignored, place it after "union" to apply attribute to type declaration}}
  __attribute__((visibility("hidden")))  __attribute__((aligned)) enum D {D}; // expected-warning{{attribute 'visibility' is ignored, place it after "enum" to apply attribute to type declaration}} \
  // expected-warning{{attribute 'aligned' is ignored, place it after "enum" to apply attribute to type declaration}}
  __attribute__((visibility("hidden")))  __attribute__((aligned)) enum class EC {}; // expected-warning{{attribute 'visibility' is ignored, place it after "enum class" to apply attribute to type declaration}} \
  // expected-warning{{attribute 'aligned' is ignored, place it after "enum class" to apply attribute to type declaration}}
  __attribute__((visibility("hidden")))  __attribute__((aligned)) enum struct ES {}; // expected-warning{{attribute 'visibility' is ignored, place it after "enum struct" to apply attribute to type declaration}} \
  // expected-warning{{attribute 'aligned' is ignored, place it after "enum struct" to apply attribute to type declaration}}

  // Test that we get the same warnings for type declarations nested in a record.
  struct X {
    __attribute__((visibility("hidden")))  __attribute__((aligned)) class A; // expected-warning{{attribute 'visibility' is ignored, place it after "class" to apply attribute to type declaration}} \
    // expected-warning{{attribute 'aligned' is ignored, place it after "class" to apply attribute to type declaration}}
    __attribute__((visibility("hidden")))  __attribute__((aligned)) struct B; // expected-warning{{attribute 'visibility' is ignored, place it after "struct" to apply attribute to type declaration}} \
    // expected-warning{{attribute 'aligned' is ignored, place it after "struct" to apply attribute to type declaration}}
    __attribute__((visibility("hidden")))  __attribute__((aligned)) union C; // expected-warning{{attribute 'visibility' is ignored, place it after "union" to apply attribute to type declaration}} \
    // expected-warning{{attribute 'aligned' is ignored, place it after "union" to apply attribute to type declaration}}
    __attribute__((visibility("hidden")))  __attribute__((aligned)) enum D {D}; // expected-warning{{attribute 'visibility' is ignored, place it after "enum" to apply attribute to type declaration}} \
    // expected-warning{{attribute 'aligned' is ignored, place it after "enum" to apply attribute to type declaration}}
    __attribute__((visibility("hidden")))  __attribute__((aligned)) enum class EC {}; // expected-warning{{attribute 'visibility' is ignored, place it after "enum class" to apply attribute to type declaration}} \
    // expected-warning{{attribute 'aligned' is ignored, place it after "enum class" to apply attribute to type declaration}}
    __attribute__((visibility("hidden")))  __attribute__((aligned)) enum struct ES {}; // expected-warning{{attribute 'visibility' is ignored, place it after "enum struct" to apply attribute to type declaration}} \
    // expected-warning{{attribute 'aligned' is ignored, place it after "enum struct" to apply attribute to type declaration}}

    // Also test [[]] attribute syntax. These attributes precede the tag, so
    // they appertain to the tag declaration; no warning is produced.
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] class E;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] struct F;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] union G;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum H {H};
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum class I {};
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum struct J {};
  };
}

namespace test2 {
  __attribute__((visibility("hidden")))  __attribute__((aligned)) class A {} a;
  __attribute__((visibility("hidden")))  __attribute__((aligned)) struct B {} b;
  __attribute__((visibility("hidden")))  __attribute__((aligned)) union C {} c;
  __attribute__((visibility("hidden")))  __attribute__((aligned)) enum D {D} d;
  __attribute__((visibility("hidden")))  __attribute__((aligned)) enum class EC {} ec;
  __attribute__((visibility("hidden")))  __attribute__((aligned)) enum struct ES {} es;

  struct X {
    __attribute__((visibility("hidden")))  __attribute__((aligned)) class A {} a;
    __attribute__((visibility("hidden")))  __attribute__((aligned)) struct B {} b;
    __attribute__((visibility("hidden")))  __attribute__((aligned)) union C {} c;
    __attribute__((visibility("hidden")))  __attribute__((aligned)) enum D {D} d;
    __attribute__((visibility("hidden")))  __attribute__((aligned)) enum class EC {} ec;
    __attribute__((visibility("hidden")))  __attribute__((aligned)) enum struct ES {} es;

    // A C++11 attribute-specifier-seq before the tag is ambiguous when the
    // declaration also declares a variable: the attributes could appertain to
    // the tag or to the variable.
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] class E {} e; // expected-error{{ambiguous attribute specifier sequence preceding 'class'}}
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] struct F {} f; // expected-error{{ambiguous attribute specifier sequence preceding 'struct'}}
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] union G {} g; // expected-error{{ambiguous attribute specifier sequence preceding 'union'}}
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum H {H} h; // expected-error{{ambiguous attribute specifier sequence preceding 'enum'}}
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum class I {} i; // expected-error{{ambiguous attribute specifier sequence preceding 'enum'}}
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum struct J {} j; // expected-error{{ambiguous attribute specifier sequence preceding 'enum'}}
  };
}
