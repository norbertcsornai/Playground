#pragma once  // codex-line-comment: documents this line.

#include "overlay/ArrowRenderer.h"  // codex-line-comment: documents this line.
#include "shared/DisplayInfo.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class OverlayWindow {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  OverlayWindow();  // codex-line-comment: documents this line.
  ~OverlayWindow();  // codex-line-comment: documents this line.

  bool create(const DisplayInfo& display);  // codex-line-comment: documents this line.
  void showArrow();  // codex-line-comment: documents this line.
  void hideArrow();  // codex-line-comment: documents this line.
  void centerOnDisplay(const DisplayInfo& display);  // codex-line-comment: documents this line.
  void ensureClickThrough();  // codex-line-comment: documents this line.
  void destroy();  // codex-line-comment: documents this line.
  void setArrowSize(const Size& size);  // codex-line-comment: documents this line.

  [[nodiscard]] bool isCreated() const;  // codex-line-comment: documents this line.
  [[nodiscard]] bool isVisible() const;  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  ArrowRenderer renderer_{};  // codex-line-comment: documents this line.
  DisplayInfo targetDisplay_{};  // codex-line-comment: documents this line.
  bool clickThrough_{true};  // codex-line-comment: documents this line.
  bool visible_{false};  // codex-line-comment: documents this line.
  void* nativeWindow_{nullptr};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
