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
  void renderTo(void* deviceContext) const;  // Paints the arrow into a device context, used by the window procedure.

  [[nodiscard]] bool isCreated() const;  // Declares isCreated and marks its return value as important.
  [[nodiscard]] bool isVisible() const;  // Reports whether the arrow is currently painted.
  [[nodiscard]] bool isWindowShown() const;  // Reports whether the underlying window is shown, which should stay constant.

 private:  // Makes the following members private implementation details.
  void redraw();  // Repaints the arrow immediately rather than waiting for a paint message.

  ArrowRenderer renderer_{};  // Declares renderer_ with value initialization.
  DisplayInfo targetDisplay_{};  // Declares targetDisplay_ with value initialization.
  Rect placement_{};  // Records the placement last applied, so unchanged positions skip SetWindowPos.
  bool clickThrough_{true};  // Declares clickThrough_ and initializes it with true.
  bool arrowVisible_{false};  // Tracks whether the arrow shape is painted, independent of window visibility.
  void* nativeWindow_{nullptr};  // Declares nativeWindow_ and initializes it with nullptr.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
