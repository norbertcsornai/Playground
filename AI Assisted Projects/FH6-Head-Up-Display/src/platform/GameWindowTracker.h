#pragma once  // Prevents this header from being included more than once.

#include <optional>  // Imports the optional standard library declarations used in this file.
#include <string>  // Imports the string standard library declarations used in this file.

#include "shared/DisplayInfo.h"  // Imports project declarations from shared/DisplayInfo.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class GameWindowTracker {  // Declares the GameWindowTracker class interface and members.
 public:  // Makes the following members part of the public API.
  explicit GameWindowTracker(std::string targetWindowTitle = "Forza Horizon");  // Declares the explicit GameWindowTracker constructor.

  // findGameWindow(), isGameRunning(), isGameVisible(), and getActiveDisplay() all read state
  // cached by the most recent refresh() call rather than re-scanning windows themselves, so a
  // caller that wants current results must call refresh() once per polling cycle first. This
  // keeps EnumWindows to a single pass per frame instead of once per accessor call.
  [[nodiscard]] std::optional<WindowInfo> findGameWindow();  // Declares findGameWindow and marks its return value as important.
  [[nodiscard]] bool isGameRunning();  // Declares isGameRunning and marks its return value as important.
  [[nodiscard]] bool isGameVisible();  // Declares isGameVisible and marks its return value as important.
  [[nodiscard]] DisplayInfo getActiveDisplay();  // Declares getActiveDisplay and marks its return value as important.
  void refresh();  // Declares function refresh for callers.

 private:  // Makes the following members private implementation details.
  std::string targetWindowTitle_;  // Declares targetWindowTitle_ for use in this scope.
  std::optional<WindowInfo> gameWindow_{};  // Declares gameWindow_ with value initialization.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
