#include "overlay/OverlayWindow.h"  // Imports project declarations from overlay/OverlayWindow.h.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
#include <windows.h>  // Imports the windows.h standard library declarations used in this file.
#endif  // Ends the compile-time selection block.

namespace fh6 {  // Places the following declarations inside namespace fh6.

namespace {  // Starts a file-local helper namespace.

constexpr int kArrowVerticalOffsetPixels = -200;  // Defines compile-time constant kArrowVerticalOffsetPixels as -200.
constexpr int kOverlayAlpha = 255;  // Defines compile-time constant kOverlayAlpha as 255.

}  // Ends the current code block.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
namespace {  // Starts a file-local helper namespace.

constexpr const char* kOverlayClassName = "FH6HudOverlayWindow";  // Defines compile-time constant kOverlayClassName as "FH6HudOverlayWindow".

LRESULT CALLBACK overlayWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {  // Begins function overlayWndProc.
  if (msg == WM_NCCREATE) {  // Guards the following work behind the window creation message.
    // Stash the owning OverlayWindow so paint messages can reach its renderer.
    auto* create = reinterpret_cast<CREATESTRUCTA*>(lparam);  // Sets auto* create to the creation parameters.
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));  // Stores the owning window pointer.
    return DefWindowProc(hwnd, msg, wparam, lparam);  // Returns DefWindowProc(hwnd, msg, wparam, lparam) to the caller.
  }  // Ends the current code block.

  if (msg == WM_NCHITTEST) {  // Guards the following work behind the condition msg == WM_NCHITTEST.
    return HTTRANSPARENT;  // Returns HTTRANSPARENT to the caller.
  }  // Ends the current code block.

  if (msg == WM_PAINT) {  // Guards the following work behind the paint message.
    // Repairs the arrow if Windows ever invalidates the overlay; the show path also paints directly
    // so the cue still appears immediately without waiting for this message to be dispatched.
    auto* owner = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));  // Sets auto* owner to the owning overlay window.
    PAINTSTRUCT paint{};  // Declares paint with value initialization.
    HDC dc = BeginPaint(hwnd, &paint);  // Sets HDC dc to BeginPaint(hwnd, &paint).
    if (owner != nullptr) {  // Guards the following work behind a known owner.
      owner->renderTo(dc);  // Calls renderTo through owner.
    }  // Ends the current code block.
    EndPaint(hwnd, &paint);  // Invokes EndPaint with the supplied arguments.
    return 0;  // Returns 0 to the caller.
  }  // Ends the current code block.

  return DefWindowProc(hwnd, msg, wparam, lparam);  // Returns DefWindowProc(hwnd, msg, wparam, lparam) to the caller.
}  // Ends the current code block.

void registerOverlayClass() {  // Begins function registerOverlayClass.
  static bool registered = false;  // Sets static bool registered to false.
  if (registered) {  // Guards the following work behind the condition registered.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  WNDCLASSEXA wc{};  // Declares wc with value initialization.
  wc.cbSize = sizeof(WNDCLASSEXA);  // Sets wc.cbSize to sizeof(WNDCLASSEXA).
  wc.lpfnWndProc = overlayWndProc;  // Sets wc.lpfnWndProc to overlayWndProc.
  wc.hInstance = GetModuleHandle(nullptr);  // Sets wc.hInstance to GetModuleHandle(nullptr).
  wc.lpszClassName = kOverlayClassName;  // Sets wc.lpszClassName to kOverlayClassName.
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);  // Sets wc.hCursor to LoadCursor(nullptr, IDC_ARROW).
  RegisterClassExA(&wc);  // Invokes RegisterClassExA with the supplied arguments.
  registered = true;  // Sets registered to true.
}  // Ends the current code block.

}  // Ends the current code block.
#endif  // Ends the compile-time selection block.

OverlayWindow::OverlayWindow() = default;  // Sets OverlayWindow::OverlayWindow() to default.

OverlayWindow::~OverlayWindow() {  // Starts a multi-line initializer or scope for OverlayWindow::~OverlayWindow().
  destroy();  // Invokes destroy with the supplied arguments.
}  // Ends the current code block.

bool OverlayWindow::create(const DisplayInfo& display) {  // Implements OverlayWindow::create.
  targetDisplay_ = display;  // Sets targetDisplay_ to display.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  registerOverlayClass();  // Invokes registerOverlayClass with the supplied arguments.
  renderer_.setSize(renderer_.size());  // Calls setSize on renderer_.

  const auto size = renderer_.size();  // Sets const auto size to renderer_.size().
  const int x = display.bounds.x + (display.bounds.width - size.width) / 2;  // Sets const int x to display.bounds.x + (display.bounds.width - size.width) / 2.
  const int y = display.bounds.y + (display.bounds.height - size.height) / 2 +  // Continues the surrounding declaration or control-flow expression.
                kArrowVerticalOffsetPixels;  // Executes kArrowVerticalOffsetPixels.

  HWND hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |  // Continues the surrounding declaration or control-flow expression.
                                  WS_EX_NOACTIVATE,  // Supplies WS_EX_NOACTIVATE to the surrounding call or initializer.
                              kOverlayClassName, "FH6 HUD Overlay", WS_POPUP, x, y, size.width,  // Supplies kOverlayClassName, "FH6 HUD Overlay", WS_POPUP, x, y, size.width to the surrounding call or initializer.
                              size.height, nullptr, nullptr, GetModuleHandle(nullptr), this);  // Passes this overlay as the creation parameter so the window procedure can reach it.
  if (hwnd == nullptr) {  // Guards the following work behind the condition hwnd == nullptr.
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), static_cast<BYTE>(kOverlayAlpha),  // Supplies SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), static_cast<BYTE>(kOverlayAlpha) to the surrounding call or initializer.
                             LWA_COLORKEY | LWA_ALPHA);  // Executes LWA_COLORKEY | LWA_ALPHA).
  nativeWindow_ = hwnd;  // Sets nativeWindow_ to hwnd.
  placement_ = Rect{x, y, size.width, size.height};  // Records the placement applied at creation.
  ensureClickThrough();  // Invokes ensureClickThrough with the supplied arguments.

  // Shown once here and never hidden again; the arrow itself is toggled by painting. Painting
  // straight away makes the window fully transparent before it can appear as an unpainted block.
  redraw();  // Paints the initial fully transparent contents.
  ShowWindow(hwnd, SW_SHOWNOACTIVATE);  // Invokes ShowWindow with the supplied arguments.
  return true;  // Returns true to the caller.
#else  // Selects this compile-time branch when earlier branches were not selected.
  nativeWindow_ = reinterpret_cast<void*>(1);  // Sets nativeWindow_ to reinterpret_cast<void*>(1).
  return !display.bounds.empty();  // Returns !display.bounds.empty() to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

// The overlay window is shown once at creation and then left shown for the process lifetime.
// Toggling a topmost layered window's visibility makes the window manager recompute z-order and
// makes the compositor re-evaluate the fullscreen game beneath it, which costs the game frames --
// badly so when the gear flickers between white and red and the cue toggles rapidly. Instead the
// arrow is shown and hidden purely by what gets painted: the window is colour-keyed on black, so a
// window filled entirely with black is completely transparent and no window state changes at all.
void OverlayWindow::showArrow() {  // Implements OverlayWindow::showArrow.
  if (!isCreated() || arrowVisible_) {  // Guards the following work behind an uncreated or already visible arrow.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  arrowVisible_ = true;  // Sets arrowVisible_ to true.
  redraw();  // Paints the arrow immediately so the cue is not delayed by message dispatch.
}  // Ends the current code block.

void OverlayWindow::hideArrow() {  // Implements OverlayWindow::hideArrow.
  if (!isCreated() || !arrowVisible_) {  // Guards the following work behind an uncreated or already hidden arrow.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  arrowVisible_ = false;  // Sets arrowVisible_ to false.
  redraw();  // Repaints as fully transparent, which removes the arrow without touching window state.
}  // Ends the current code block.

void OverlayWindow::centerOnDisplay(const DisplayInfo& display) {  // Implements OverlayWindow::centerOnDisplay.
  targetDisplay_ = display;  // Sets targetDisplay_ to display.
  if (!isCreated()) {  // Guards the following work behind the condition !isCreated().
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  const auto size = renderer_.size();  // Sets const auto size to renderer_.size().
  const int x = display.bounds.x + (display.bounds.width - size.width) / 2;  // Sets const int x to display.bounds.x + (display.bounds.width - size.width) / 2.
  const int y = display.bounds.y + (display.bounds.height - size.height) / 2 +  // Continues the surrounding declaration or control-flow expression.
                kArrowVerticalOffsetPixels;  // Executes kArrowVerticalOffsetPixels.

  const Rect placement{x, y, size.width, size.height};  // Declares placement describing where the overlay should sit.
  if (placement.x == placement_.x && placement.y == placement_.y &&  // Guards the following work behind an unchanged placement.
      placement.width == placement_.width && placement.height == placement_.height) {  // Continues the unchanged placement check.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.
  placement_ = placement;  // Sets placement_ to placement.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  SetWindowPos(static_cast<HWND>(nativeWindow_), HWND_TOPMOST, x, y, size.width, size.height,  // Supplies SetWindowPos(static_cast<HWND>(nativeWindow_), HWND_TOPMOST, x, y, size.width, size.height to the surrounding call or initializer.
               SWP_NOACTIVATE);  // Executes SWP_NOACTIVATE).
#endif  // Ends the compile-time selection block.
  redraw();  // Repaints after a move, since moving the window can leave its contents stale.
}  // Ends the current code block.

void OverlayWindow::redraw() {  // Implements OverlayWindow::redraw.
  if (!isCreated()) {  // Guards the following work behind the condition !isCreated().
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  HWND hwnd = static_cast<HWND>(nativeWindow_);  // Sets HWND hwnd to static_cast<HWND>(nativeWindow_).
  HDC dc = GetDC(hwnd);  // Sets HDC dc to GetDC(hwnd).
  renderTo(dc);  // Calls renderTo with the freshly acquired device context.
  ReleaseDC(hwnd, dc);  // Invokes ReleaseDC with the supplied arguments.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

void OverlayWindow::renderTo(void* deviceContext) const {  // Implements OverlayWindow::renderTo.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  auto* dc = static_cast<HDC>(deviceContext);  // Sets auto* dc to static_cast<HDC>(deviceContext).
  if (dc == nullptr || !isCreated()) {  // Guards the following work behind a missing context or window.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  // The window is colour-keyed on black, so filling with black clears to fully transparent. When
  // the arrow is hidden that is all this does, leaving an invisible window rather than a hidden one.
  RECT client{};  // Declares client with value initialization.
  GetClientRect(static_cast<HWND>(nativeWindow_), &client);  // Invokes GetClientRect with the supplied arguments.
  HBRUSH clearBrush = CreateSolidBrush(RGB(0, 0, 0));  // Sets HBRUSH clearBrush to CreateSolidBrush(RGB(0, 0, 0)).
  FillRect(dc, &client, clearBrush);  // Invokes FillRect with the supplied arguments.
  DeleteObject(clearBrush);  // Invokes DeleteObject with the supplied arguments.
  if (arrowVisible_) {  // Guards the arrow paint behind the arrow being currently shown.
    renderer_.render(dc);  // Calls render on renderer_.
  }  // Ends the current code block.
#else  // Selects this compile-time branch when earlier branches were not selected.
  (void)deviceContext;  // Marks deviceContext as intentionally unused in this build path.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

void OverlayWindow::ensureClickThrough() {  // Implements OverlayWindow::ensureClickThrough.
  clickThrough_ = true;  // Sets clickThrough_ to true.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  if (!isCreated()) {  // Guards the following work behind the condition !isCreated().
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  HWND hwnd = static_cast<HWND>(nativeWindow_);  // Sets HWND hwnd to static_cast<HWND>(nativeWindow_).
  LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);  // Sets LONG_PTR style to GetWindowLongPtr(hwnd, GWL_EXSTYLE).
  style |= WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;  // Sets style | to WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW.
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);  // Invokes SetWindowLongPtr with the supplied arguments.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

void OverlayWindow::destroy() {  // Implements OverlayWindow::destroy.
  if (!isCreated()) {  // Guards the following work behind the condition !isCreated().
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  DestroyWindow(static_cast<HWND>(nativeWindow_));  // Invokes DestroyWindow with the supplied arguments.
#endif  // Ends the compile-time selection block.
  nativeWindow_ = nullptr;  // Sets nativeWindow_ to nullptr.
  arrowVisible_ = false;  // Sets arrowVisible_ to false.
  placement_ = Rect{};  // Clears the recorded placement so a recreated overlay repositions itself.
}  // Ends the current code block.

void OverlayWindow::setArrowSize(const Size& size) {  // Implements OverlayWindow::setArrowSize.
  renderer_.setSize(size);  // Calls setSize on renderer_.
}  // Ends the current code block.

bool OverlayWindow::isCreated() const {  // Implements OverlayWindow::isCreated.
  return nativeWindow_ != nullptr;  // Returns nativeWindow_ != nullptr to the caller.
}  // Ends the current code block.

bool OverlayWindow::isVisible() const {  // Implements OverlayWindow::isVisible.
  return arrowVisible_;  // Returns arrowVisible_ to the caller.
}  // Ends the current code block.

bool OverlayWindow::isWindowShown() const {  // Implements OverlayWindow::isWindowShown.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  return isCreated() && IsWindowVisible(static_cast<HWND>(nativeWindow_)) != 0;  // Returns whether the underlying window is currently shown.
#else  // Selects this compile-time branch when earlier branches were not selected.
  return isCreated();  // Returns isCreated() to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

}  // Ends the current code block.
