#pragma once  // codex-line-comment: documents this line.

#include <optional>  // codex-line-comment: documents this line.
#include <string>  // codex-line-comment: documents this line.

#include "shared/DisplayInfo.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class GameWindowTracker {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  explicit GameWindowTracker(std::string targetWindowTitle = "Forza Horizon");  // codex-line-comment: documents this line.

  [[nodiscard]] std::optional<WindowInfo> findGameWindow();  // codex-line-comment: documents this line.
  [[nodiscard]] bool isGameRunning();  // codex-line-comment: documents this line.
  [[nodiscard]] bool isGameVisible();  // codex-line-comment: documents this line.
  [[nodiscard]] DisplayInfo getActiveDisplay();  // codex-line-comment: documents this line.
  void refresh();  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  std::string targetWindowTitle_;  // codex-line-comment: documents this line.
  std::optional<WindowInfo> gameWindow_{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
