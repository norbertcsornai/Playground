#pragma once

#include "overlay/ArrowRenderer.h"
#include "shared/DisplayInfo.h"
#include "shared/Geometry.h"

namespace fh6 {

class OverlayWindow {
 public:
  OverlayWindow();
  ~OverlayWindow();

  bool create(const DisplayInfo& display);
  void showArrow();
  void hideArrow();
  void centerOnDisplay(const DisplayInfo& display);
  void ensureClickThrough();
  void destroy();
  void setArrowSize(const Size& size);

  [[nodiscard]] bool isCreated() const;
  [[nodiscard]] bool isVisible() const;

 private:
  ArrowRenderer renderer_{};
  DisplayInfo targetDisplay_{};
  bool clickThrough_{true};
  bool visible_{false};
  void* nativeWindow_{nullptr};
};

}  // namespace fh6
