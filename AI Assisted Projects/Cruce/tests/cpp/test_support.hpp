#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

inline void expect(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename Left, typename Right>
void expect_eq(const Left& left, const Right& right, const std::string& message) {
  if (!(left == right)) {
    throw std::runtime_error(message);
  }
}

inline int run_test(const std::string& name, const std::function<void()>& test) {
  try {
    test();
    std::cout << "[PASS] " << name << "\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "[FAIL] " << name << ": " << error.what() << "\n";
    return 1;
  }
}
