#include "platform/GameWindowTracker.h"

#ifdef _WIN32
#include <array>
#include <windows.h>
#endif

namespace fh6 {

GameWindowTracker::GameWindowTracker(std::string targetWindowTitle)
    : targetWindowTitle_(std::move(targetWindowTitle)) {}

std::optional<WindowInfo> GameWindowTracker::findGameWindow() {
  refresh();
  return gameWindow_;
}

bool GameWindowTracker::isGameRunning() {
  refresh();
  return gameWindow_.has_value();
}

bool GameWindowTracker::isGameVisible() {
  refresh();
  if (!gameWindow_) {
    return false;
  }

#ifdef _WIN32
  HWND hwnd = static_cast<HWND>(gameWindow_->handle);
  return IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd);
#else
  return false;
#endif
}

DisplayInfo GameWindowTracker::getActiveDisplay() {
  refresh();
  return gameWindow_ ? gameWindow_->display : DisplayInfo{};
}

void GameWindowTracker::refresh() {
#ifdef _WIN32
  struct SearchContext {
    const std::string* title;
    std::optional<WindowInfo>* result;
  } context{&targetWindowTitle_, &gameWindow_};

  gameWindow_.reset();

  EnumWindows(
      [](HWND hwnd, LPARAM lparam) -> BOOL {
        auto* context = reinterpret_cast<SearchContext*>(lparam);
        if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) {
          return TRUE;
        }

        std::array<char, 512> title{};
        GetWindowTextA(hwnd, title.data(), static_cast<int>(title.size()));
        const std::string windowTitle(title.data());
        if (windowTitle.find(*context->title) == std::string::npos) {
          return TRUE;
        }

        RECT rect{};
        GetWindowRect(hwnd, &rect);

        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(monitor, &monitorInfo);

        DisplayInfo display{
            "active",
            Rect{monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top},
            1.0F,
            false,
        };

        *context->result = WindowInfo{
            hwnd,
            windowTitle,
            Rect{rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top},
            display,
        };
        return FALSE;
      },
      reinterpret_cast<LPARAM>(&context));
#else
  gameWindow_.reset();
#endif
}

}  // namespace fh6
