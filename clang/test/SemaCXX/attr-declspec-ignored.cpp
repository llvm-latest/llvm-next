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

    // Also test [[]] attribute syntax. Prefix attribute-specifier-seqs now
    // appertain to the tag, so these produce neither a "misplaced attributes"
    // error nor an "ignored, place it after ..." warning.
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] class E; // no diagnostic
    // (line kept to preserve relative verify directives below)
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] struct F; // no diagnostic
    // (line kept to preserve relative verify directives below)
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] union G; // no diagnostic
    // (line kept to preserve relative verify directives below)
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum H {H}; // no diagnostic
    // (line kept to preserve relative verify directives below)
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum class I {}; // no diagnostic
    // (line kept to preserve relative verify directives below)
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum struct J {}; // no diagnostic
    // (line kept to preserve relative verify directives below)
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

    [[gnu::visibility("hidden")]]  [[gnu::aligned]] class E {} e;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] struct F {} f;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] union G {} g;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum H {H} h;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum class I {} i;
    [[gnu::visibility("hidden")]]  [[gnu::aligned]] enum struct J {} j;
  };
}
