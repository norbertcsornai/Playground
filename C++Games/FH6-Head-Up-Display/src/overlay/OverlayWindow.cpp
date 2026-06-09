#include "overlay/OverlayWindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace fh6 {

namespace {

constexpr int kArrowVerticalOffsetPixels = -200;

}  // namespace

#ifdef _WIN32
namespace {

constexpr const char* kOverlayClassName = "FH6HudOverlayWindow";

LRESULT CALLBACK overlayWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_NCHITTEST) {
    return HTTRANSPARENT;
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

void registerOverlayClass() {
  static bool registered = false;
  if (registered) {
    return;
  }

  WNDCLASSEXA wc{};
  wc.cbSize = sizeof(WNDCLASSEXA);
  wc.lpfnWndProc = overlayWndProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = kOverlayClassName;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassExA(&wc);
  registered = true;
}

}  // namespace
#endif

OverlayWindow::OverlayWindow() = default;

OverlayWindow::~OverlayWindow() {
  destroy();
}

bool OverlayWindow::create(const DisplayInfo& display) {
  targetDisplay_ = display;
#ifdef _WIN32
  registerOverlayClass();
  renderer_.setSize(renderer_.size());

  const auto size = renderer_.size();
  const int x = display.bounds.x + (display.bounds.width - size.width) / 2;
  const int y = display.bounds.y + (display.bounds.height - size.height) / 2 +
                kArrowVerticalOffsetPixels;

  HWND hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |
                                 WS_EX_NOACTIVATE,
                             kOverlayClassName, "FH6 HUD Overlay", WS_POPUP, x, y, size.width,
                             size.height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
  if (hwnd == nullptr) {
    return false;
  }

  SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
  nativeWindow_ = hwnd;
  ensureClickThrough();
  return true;
#else
  nativeWindow_ = reinterpret_cast<void*>(1);
  return !display.bounds.empty();
#endif
}

void OverlayWindow::showArrow() {
  if (!isCreated()) {
    return;
  }

#ifdef _WIN32
  HWND hwnd = static_cast<HWND>(nativeWindow_);
  HDC dc = GetDC(hwnd);
  RECT client{};
  GetClientRect(hwnd, &client);
  HBRUSH clearBrush = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(dc, &client, clearBrush);
  DeleteObject(clearBrush);
  renderer_.render(dc);
  ReleaseDC(hwnd, dc);
  ShowWindow(hwnd, SW_SHOWNOACTIVATE);
#endif
  visible_ = true;
}

void OverlayWindow::hideArrow() {
  if (!isCreated()) {
    return;
  }

#ifdef _WIN32
  ShowWindow(static_cast<HWND>(nativeWindow_), SW_HIDE);
#endif
  visible_ = false;
}

void OverlayWindow::centerOnDisplay(const DisplayInfo& display) {
  targetDisplay_ = display;
  if (!isCreated()) {
    return;
  }

  const auto size = renderer_.size();
  const int x = display.bounds.x + (display.bounds.width - size.width) / 2;
  const int y = display.bounds.y + (display.bounds.height - size.height) / 2 +
                kArrowVerticalOffsetPixels;

#ifdef _WIN32
  SetWindowPos(static_cast<HWND>(nativeWindow_), HWND_TOPMOST, x, y, size.width, size.height,
               SWP_NOACTIVATE);
#endif
}

void OverlayWindow::ensureClickThrough() {
  clickThrough_ = true;
#ifdef _WIN32
  if (!isCreated()) {
    return;
  }

  HWND hwnd = static_cast<HWND>(nativeWindow_);
  LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
  style |= WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
#endif
}

void OverlayWindow::destroy() {
  if (!isCreated()) {
    return;
  }

#ifdef _WIN32
  DestroyWindow(static_cast<HWND>(nativeWindow_));
#endif
  nativeWindow_ = nullptr;
  visible_ = false;
}

void OverlayWindow::setArrowSize(const Size& size) {
  renderer_.setSize(size);
}

bool OverlayWindow::isCreated() const {
  return nativeWindow_ != nullptr;
}

bool OverlayWindow::isVisible() const {
  return visible_;
}

}  // namespace fh6
