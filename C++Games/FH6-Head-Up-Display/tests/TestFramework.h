#pragma once  // codex-line-comment: documents this line.

#include <functional>  // codex-line-comment: documents this line.
#include <iostream>  // codex-line-comment: documents this line.
#include <stdexcept>  // codex-line-comment: documents this line.
#include <string>  // codex-line-comment: documents this line.
#include <string_view>  // codex-line-comment: documents this line.
#include <vector>  // codex-line-comment: documents this line.

namespace fh6::test {  // codex-line-comment: documents this line.

using TestFn = std::function<void()>;  // codex-line-comment: documents this line.

struct TestCase {  // codex-line-comment: documents this line.
  std::string name;  // codex-line-comment: documents this line.
  TestFn fn;  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

inline std::vector<TestCase>& registry() {  // codex-line-comment: documents this line.
  static std::vector<TestCase> tests;  // codex-line-comment: documents this line.
  return tests;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

struct Registrar {  // codex-line-comment: documents this line.
  Registrar(std::string name, TestFn fn) {  // codex-line-comment: documents this line.
    registry().push_back(TestCase{std::move(name), std::move(fn)});  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

inline void require(bool condition, std::string_view message) {  // codex-line-comment: documents this line.
  if (!condition) {  // codex-line-comment: documents this line.
    throw std::runtime_error(std::string(message));  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

inline int runAll() {  // codex-line-comment: documents this line.
  int failed = 0;  // codex-line-comment: documents this line.
  for (const auto& test : registry()) {  // codex-line-comment: documents this line.
    try {  // codex-line-comment: documents this line.
      test.fn();  // codex-line-comment: documents this line.
      std::cout << "[PASS] " << test.name << '\n';  // codex-line-comment: documents this line.
    } catch (const std::exception& ex) {  // codex-line-comment: documents this line.
      ++failed;  // codex-line-comment: documents this line.
      std::cerr << "[FAIL] " << test.name << ": " << ex.what() << '\n';  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::cout << registry().size() - static_cast<std::size_t>(failed) << "/" << registry().size()  // codex-line-comment: documents this line.
            << " tests passed\n";  // codex-line-comment: documents this line.
  return failed == 0 ? 0 : 1;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6::test  // codex-line-comment: documents this line.

#define FH6_TEST(name) /* codex-line-comment: documents this line. */                                                                  \
  static void name(); /* codex-line-comment: documents this line. */                                                                   \
  static const ::fh6::test::Registrar name##_registrar(#name, [] { name(); }); /* codex-line-comment: documents this line. */          \
  static void name()  // codex-line-comment: documents this line.

#define FH6_REQUIRE(condition) ::fh6::test::require((condition), #condition)  // codex-line-comment: documents this line.
