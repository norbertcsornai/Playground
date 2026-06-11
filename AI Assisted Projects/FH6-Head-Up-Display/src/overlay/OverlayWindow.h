#pragma once  // Prevents this header from being included more than once.

#include "overlay/ArrowRenderer.h"  // Imports project declarations from overlay/ArrowRenderer.h.
#include "shared/DisplayInfo.h"  // Imports project declarations from shared/DisplayInfo.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class OverlayWindow {  // Declares the OverlayWindow class interface and members.
 public:  // Makes the following members part of the public API.
  OverlayWindow();  // Invokes OverlayWindow with the supplied arguments.
  ~OverlayWindow();  // Executes ~OverlayWindow().

  bool create(const DisplayInfo& display);  // Declares function create for callers.
  void showArrow();  // Declares function showArrow for callers.
  void hideArrow();  // Declares function hideArrow for callers.
  void centerOnDisplay(const DisplayInfo& display);  // Declares function centerOnDisplay for callers.
  void ensureClickThrough();  // Declares function ensureClickThrough for callers.
  void destroy();  // Declares function destroy for callers.
  void setArrowSize(const Size& size);  // Declares function setArrowSize for callers.

  [[nodiscard]] bool isCreated() const;  // Declares isCreated and marks its return value as important.
  [[nodiscard]] bool isVisible() const;  // Declares isVisible and marks its return value as important.

 private:  // Makes the following members private implementation details.
  ArrowRenderer renderer_{};  // Declares renderer_ with value initialization.
  DisplayInfo targetDisplay_{};  // Declares targetDisplay_ with value initialization.
  bool clickThrough_{true};  // Declares clickThrough_ and initializes it with true.
  bool visible_{false};  // Declares visible_ and initializes it with false.
  void* nativeWindow_{nullptr};  // Declares nativeWindow_ and initializes it with nullptr.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
