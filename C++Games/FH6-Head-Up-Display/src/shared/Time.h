#pragma once

#include <chrono>

namespace fh6 {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

}  // namespace fh6
