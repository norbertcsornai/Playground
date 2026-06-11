#pragma once  // Prevents this header from being included more than once.

#include <chrono>  // Imports the chrono standard library declarations used in this file.

namespace fh6 {  // Places the following declarations inside namespace fh6.

using Clock = std::chrono::steady_clock;  // Aliases Clock to std::chrono::steady_clock.
using TimePoint = Clock::time_point;  // Aliases TimePoint to Clock::time_point.

}  // Ends the current code block.
