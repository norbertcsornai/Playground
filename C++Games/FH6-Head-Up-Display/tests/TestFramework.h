#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fh6::test {

using TestFn = std::function<void()>;

struct TestCase {
  std::string name;
  TestFn fn;
};

inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> tests;
  return tests;
}

struct Registrar {
  Registrar(std::string name, TestFn fn) {
    registry().push_back(TestCase{std::move(name), std::move(fn)});
  }
};

inline void require(bool condition, std::string_view message) {
  if (!condition) {
    throw std::runtime_error(std::string(message));
  }
}

inline int runAll() {
  int failed = 0;
  for (const auto& test : registry()) {
    try {
      test.fn();
      std::cout << "[PASS] " << test.name << '\n';
    } catch (const std::exception& ex) {
      ++failed;
      std::cerr << "[FAIL] " << test.name << ": " << ex.what() << '\n';
    }
  }

  std::cout << registry().size() - static_cast<std::size_t>(failed) << "/" << registry().size()
            << " tests passed\n";
  return failed == 0 ? 0 : 1;
}

}  // namespace fh6::test

#define FH6_TEST(name)                                                                  \
  static void name();                                                                   \
  static const ::fh6::test::Registrar name##_registrar(#name, [] { name(); });          \
  static void name()

#define FH6_REQUIRE(condition) ::fh6::test::require((condition), #condition)
