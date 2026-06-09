#include "platform/GameWindowTracker.h"  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
#include <array>  // codex-line-comment: documents this line.
#include <windows.h>  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

GameWindowTracker::GameWindowTracker(std::string targetWindowTitle)  // codex-line-comment: documents this line.
    : targetWindowTitle_(std::move(targetWindowTitle)) {}  // codex-line-comment: documents this line.

std::optional<WindowInfo> GameWindowTracker::findGameWindow() {  // codex-line-comment: documents this line.
  refresh();  // codex-line-comment: documents this line.
  return gameWindow_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool GameWindowTracker::isGameRunning() {  // codex-line-comment: documents this line.
  refresh();  // codex-line-comment: documents this line.
  return gameWindow_.has_value();  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool GameWindowTracker::isGameVisible() {  // codex-line-comment: documents this line.
  refresh();  // codex-line-comment: documents this line.
  if (!gameWindow_) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
  HWND hwnd = static_cast<HWND>(gameWindow_->handle);  // codex-line-comment: documents this line.
  return IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd);  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  return false;  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

DisplayInfo GameWindowTracker::getActiveDisplay() {  // codex-line-comment: documents this line.
  refresh();  // codex-line-comment: documents this line.
  return gameWindow_ ? gameWindow_->display : DisplayInfo{};  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void GameWindowTracker::refresh() {  // codex-line-comment: documents this line.
#ifdef _WIN32  // codex-line-comment: documents this line.
  struct SearchContext {  // codex-line-comment: documents this line.
    const std::string* title;  // codex-line-comment: documents this line.
    std::optional<WindowInfo>* result;  // codex-line-comment: documents this line.
  } context{&targetWindowTitle_, &gameWindow_};  // codex-line-comment: documents this line.

  gameWindow_.reset();  // codex-line-comment: documents this line.

  EnumWindows(  // codex-line-comment: documents this line.
      [](HWND hwnd, LPARAM lparam) -> BOOL {  // codex-line-comment: documents this line.
        auto* context = reinterpret_cast<SearchContext*>(lparam);  // codex-line-comment: documents this line.
        if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) {  // codex-line-comment: documents this line.
          return TRUE;  // codex-line-comment: documents this line.
        }  // codex-line-comment: documents this line.

        std::array<char, 512> title{};  // codex-line-comment: documents this line.
        GetWindowTextA(hwnd, title.data(), static_cast<int>(title.size()));  // codex-line-comment: documents this line.
        const std::string windowTitle(title.data());  // codex-line-comment: documents this line.
        if (windowTitle.find(*context->title) == std::string::npos) {  // codex-line-comment: documents this line.
          return TRUE;  // codex-line-comment: documents this line.
        }  // codex-line-comment: documents this line.

        RECT rect{};  // codex-line-comment: documents this line.
        GetWindowRect(hwnd, &rect);  // codex-line-comment: documents this line.

        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);  // codex-line-comment: documents this line.
        MONITORINFO monitorInfo{};  // codex-line-comment: documents this line.
        monitorInfo.cbSize = sizeof(MONITORINFO);  // codex-line-comment: documents this line.
        GetMonitorInfo(monitor, &monitorInfo);  // codex-line-comment: documents this line.

        DisplayInfo display{  // codex-line-comment: documents this line.
            "active",  // codex-line-comment: documents this line.
            Rect{monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,  // codex-line-comment: documents this line.
                 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,  // codex-line-comment: documents this line.
                 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top},  // codex-line-comment: documents this line.
            1.0F,  // codex-line-comment: documents this line.
            false,  // codex-line-comment: documents this line.
        };  // codex-line-comment: documents this line.

        *context->result = WindowInfo{  // codex-line-comment: documents this line.
            hwnd,  // codex-line-comment: documents this line.
            windowTitle,  // codex-line-comment: documents this line.
            Rect{rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top},  // codex-line-comment: documents this line.
            display,  // codex-line-comment: documents this line.
        };  // codex-line-comment: documents this line.
        return FALSE;  // codex-line-comment: documents this line.
      },  // codex-line-comment: documents this line.
      reinterpret_cast<LPARAM>(&context));  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  gameWindow_.reset();  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
