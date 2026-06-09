#pragma once

#include <optional>
#include <string>

#include "shared/DisplayInfo.h"

namespace fh6 {

class GameWindowTracker {
 public:
  explicit GameWindowTracker(std::string targetWindowTitle = "Forza Horizon");

  [[nodiscard]] std::optional<WindowInfo> findGameWindow();
  [[nodiscard]] bool isGameRunning();
  [[nodiscard]] bool isGameVisible();
  [[nodiscard]] DisplayInfo getActiveDisplay();
  void refresh();

 private:
  std::string targetWindowTitle_;
  std::optional<WindowInfo> gameWindow_{};
};

}  // namespace fh6
