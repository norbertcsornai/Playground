#pragma once  // Prevents this header from being included more than once.

#include <functional>  // Imports the functional standard library declarations used in this file.
#include <iostream>  // Imports the iostream standard library declarations used in this file.
#include <stdexcept>  // Imports the stdexcept standard library declarations used in this file.
#include <string>  // Imports the string standard library declarations used in this file.
#include <string_view>  // Imports the string_view standard library declarations used in this file.
#include <vector>  // Imports the vector standard library declarations used in this file.

namespace fh6::test {  // Places the following declarations inside namespace fh6::test.

using TestFn = std::function<void()>;  // Aliases TestFn to std::function<void()>.

struct TestCase {  // Declares the TestCase value type and fields.
  std::string name;  // Declares name for use in this scope.
  TestFn fn;  // Declares fn for use in this scope.
};  // Ends the current type, struct, or initializer declaration.

inline std::vector<TestCase>& registry() {  // Begins function registry.
  static std::vector<TestCase> tests;  // Declares tests for use in this scope.
  return tests;  // Returns tests to the caller.
}  // Ends the current code block.

struct Registrar {  // Declares the Registrar value type and fields.
  Registrar(std::string name, TestFn fn) {  // Starts a multi-line initializer or scope for Registrar(std::string name, TestFn fn).
    registry().push_back(TestCase{std::move(name), std::move(fn)});  // Invokes registry with the supplied arguments.
  }  // Ends the current code block.
};  // Ends the current type, struct, or initializer declaration.

inline void require(bool condition, std::string_view message) {  // Begins function require.
  if (!condition) {  // Guards the following work behind the condition !condition.
    throw std::runtime_error(std::string(message));  // Reports failure by throwing std::runtime_error(std::string(message)).
  }  // Ends the current code block.
}  // Ends the current code block.

inline int runAll() {  // Begins function runAll.
  int failed = 0;  // Sets int failed to 0.
  for (const auto& test : registry()) {  // Iterates with loop control const auto& test : registry().
    try {  // Begins work that may throw an exception.
      test.fn();  // Calls fn on test.
      std::cout << "[PASS] " << test.name << '\n';  // Writes "[PASS] " << test.name << '\n' to standard output.
    } catch (const std::exception& ex) {  // Handles the exception object declared as const std::exception& ex.
      ++failed;  // Increments failed by one.
      std::cerr << "[FAIL] " << test.name << ": " << ex.what() << '\n';  // Writes "[FAIL] " << test.name << ": " << ex.what() << '\n' to standard error.
    }  // Ends the current code block.
  }  // Ends the current code block.

  std::cout << registry().size() - static_cast<std::size_t>(failed) << "/" << registry().size()  // Continues the surrounding declaration or control-flow expression.
            << " tests passed\n";  // Executes << " tests passed\n".
  return failed == 0 ? 0 : 1;  // Returns failed == 0 ? 0 : 1 to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

#define FH6_TEST(name) static void name(); static const ::fh6::test::Registrar name##_registrar(#name, [] { name(); }); static void name()  // Registers a named test and expands into the test function definition.

#define FH6_REQUIRE(condition) ::fh6::test::require((condition), #condition)  // Checks a test condition and reports the expression if it fails.
