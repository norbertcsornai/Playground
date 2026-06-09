#include "overlay/OverlayWindow.h"  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
#include <windows.h>  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

constexpr int kArrowVerticalOffsetPixels = -200;  // codex-line-comment: documents this line.
constexpr int kOverlayAlpha = 255;  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
namespace {  // codex-line-comment: documents this line.

constexpr const char* kOverlayClassName = "FH6HudOverlayWindow";  // codex-line-comment: documents this line.

LRESULT CALLBACK overlayWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {  // codex-line-comment: documents this line.
  if (msg == WM_NCHITTEST) {  // codex-line-comment: documents this line.
    return HTTRANSPARENT;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  return DefWindowProc(hwnd, msg, wparam, lparam);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void registerOverlayClass() {  // codex-line-comment: documents this line.
  static bool registered = false;  // codex-line-comment: documents this line.
  if (registered) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  WNDCLASSEXA wc{};  // codex-line-comment: documents this line.
  wc.cbSize = sizeof(WNDCLASSEXA);  // codex-line-comment: documents this line.
  wc.lpfnWndProc = overlayWndProc;  // codex-line-comment: documents this line.
  wc.hInstance = GetModuleHandle(nullptr);  // codex-line-comment: documents this line.
  wc.lpszClassName = kOverlayClassName;  // codex-line-comment: documents this line.
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);  // codex-line-comment: documents this line.
  RegisterClassExA(&wc);  // codex-line-comment: documents this line.
  registered = true;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

OverlayWindow::OverlayWindow() = default;  // codex-line-comment: documents this line.

OverlayWindow::~OverlayWindow() {  // codex-line-comment: documents this line.
  destroy();  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool OverlayWindow::create(const DisplayInfo& display) {  // codex-line-comment: documents this line.
  targetDisplay_ = display;  // codex-line-comment: documents this line.
#ifdef _WIN32  // codex-line-comment: documents this line.
  registerOverlayClass();  // codex-line-comment: documents this line.
  renderer_.setSize(renderer_.size());  // codex-line-comment: documents this line.

  const auto size = renderer_.size();  // codex-line-comment: documents this line.
  const int x = display.bounds.x + (display.bounds.width - size.width) / 2;  // codex-line-comment: documents this line.
  const int y = display.bounds.y + (display.bounds.height - size.height) / 2 +  // codex-line-comment: documents this line.
                kArrowVerticalOffsetPixels;  // codex-line-comment: documents this line.

  HWND hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |  // codex-line-comment: documents this line.
                                  WS_EX_NOACTIVATE,  // codex-line-comment: documents this line.
                              kOverlayClassName, "FH6 HUD Overlay", WS_POPUP, x, y, size.width,  // codex-line-comment: documents this line.
                              size.height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);  // codex-line-comment: documents this line.
  if (hwnd == nullptr) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), static_cast<BYTE>(kOverlayAlpha),  // codex-line-comment: documents this line.
                             LWA_COLORKEY | LWA_ALPHA);  // codex-line-comment: documents this line.
  nativeWindow_ = hwnd;  // codex-line-comment: documents this line.
  ensureClickThrough();  // codex-line-comment: documents this line.
  return true;  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  nativeWindow_ = reinterpret_cast<void*>(1);  // codex-line-comment: documents this line.
  return !display.bounds.empty();  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void OverlayWindow::showArrow() {  // codex-line-comment: documents this line.
  if (!isCreated()) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
  HWND hwnd = static_cast<HWND>(nativeWindow_);  // codex-line-comment: documents this line.
  HDC dc = GetDC(hwnd);  // codex-line-comment: documents this line.
  RECT client{};  // codex-line-comment: documents this line.
  GetClientRect(hwnd, &client);  // codex-line-comment: documents this line.
  HBRUSH clearBrush = CreateSolidBrush(RGB(0, 0, 0));  // codex-line-comment: documents this line.
  FillRect(dc, &client, clearBrush);  // codex-line-comment: documents this line.
  DeleteObject(clearBrush);  // codex-line-comment: documents this line.
  renderer_.render(dc);  // codex-line-comment: documents this line.
  ReleaseDC(hwnd, dc);  // codex-line-comment: documents this line.
  ShowWindow(hwnd, SW_SHOWNOACTIVATE);  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
  visible_ = true;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void OverlayWindow::hideArrow() {  // codex-line-comment: documents this line.
  if (!isCreated()) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
  ShowWindow(static_cast<HWND>(nativeWindow_), SW_HIDE);  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
  visible_ = false;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void OverlayWindow::centerOnDisplay(const DisplayInfo& display) {  // codex-line-comment: documents this line.
  targetDisplay_ = display;  // codex-line-comment: documents this line.
  if (!isCreated()) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const auto size = renderer_.size();  // codex-line-comment: documents this line.
  const int x = display.bounds.x + (display.bounds.width - size.width) / 2;  // codex-line-comment: documents this line.
  const int y = display.bounds.y + (display.bounds.height - size.height) / 2 +  // codex-line-comment: documents this line.
                kArrowVerticalOffsetPixels;  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
  SetWindowPos(static_cast<HWND>(nativeWindow_), HWND_TOPMOST, x, y, size.width, size.height,  // codex-line-comment: documents this line.
               SWP_NOACTIVATE);  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void OverlayWindow::ensureClickThrough() {  // codex-line-comment: documents this line.
  clickThrough_ = true;  // codex-line-comment: documents this line.
#ifdef _WIN32  // codex-line-comment: documents this line.
  if (!isCreated()) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  HWND hwnd = static_cast<HWND>(nativeWindow_);  // codex-line-comment: documents this line.
  LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);  // codex-line-comment: documents this line.
  style |= WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;  // codex-line-comment: documents this line.
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void OverlayWindow::destroy() {  // codex-line-comment: documents this line.
  if (!isCreated()) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
  DestroyWindow(static_cast<HWND>(nativeWindow_));  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
  nativeWindow_ = nullptr;  // codex-line-comment: documents this line.
  visible_ = false;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void OverlayWindow::setArrowSize(const Size& size) {  // codex-line-comment: documents this line.
  renderer_.setSize(size);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool OverlayWindow::isCreated() const {  // codex-line-comment: documents this line.
  return nativeWindow_ != nullptr;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool OverlayWindow::isVisible() const {  // codex-line-comment: documents this line.
  return visible_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
