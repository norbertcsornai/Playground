#include "platform/GameWindowTracker.h"  // Imports project declarations from platform/GameWindowTracker.h.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
#include <array>  // Imports the array standard library declarations used in this file.
#include <windows.h>  // Imports the windows.h standard library declarations used in this file.
#endif  // Ends the compile-time selection block.

namespace fh6 {  // Places the following declarations inside namespace fh6.

GameWindowTracker::GameWindowTracker(std::string targetWindowTitle)  // Begins the multi-line constructor definition for GameWindowTracker.
    : targetWindowTitle_(std::move(targetWindowTitle)) {}  // Initializes constructor members with targetWindowTitle_(std::move(targetWindowTitle)).

std::optional<WindowInfo> GameWindowTracker::findGameWindow() {  // Implements GameWindowTracker::findGameWindow.
  refresh();  // Invokes refresh with the supplied arguments.
  return gameWindow_;  // Returns gameWindow_ to the caller.
}  // Ends the current code block.

bool GameWindowTracker::isGameRunning() {  // Implements GameWindowTracker::isGameRunning.
  refresh();  // Invokes refresh with the supplied arguments.
  return gameWindow_.has_value();  // Returns gameWindow_.has_value() to the caller.
}  // Ends the current code block.

bool GameWindowTracker::isGameVisible() {  // Implements GameWindowTracker::isGameVisible.
  refresh();  // Invokes refresh with the supplied arguments.
  if (!gameWindow_) {  // Guards the following work behind the condition !gameWindow_.
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  HWND hwnd = static_cast<HWND>(gameWindow_->handle);  // Sets HWND hwnd to static_cast<HWND>(gameWindow_->handle).
  return IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd);  // Returns IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd) to the caller.
#else  // Selects this compile-time branch when earlier branches were not selected.
  return false;  // Returns false to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

DisplayInfo GameWindowTracker::getActiveDisplay() {  // Implements GameWindowTracker::getActiveDisplay.
  refresh();  // Invokes refresh with the supplied arguments.
  return gameWindow_ ? gameWindow_->display : DisplayInfo{};  // Returns gameWindow_ ? gameWindow_->display : DisplayInfo{} to the caller.
}  // Ends the current code block.

void GameWindowTracker::refresh() {  // Implements GameWindowTracker::refresh.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  struct SearchContext {  // Declares the SearchContext value type and fields.
    const std::string* title;  // Declares title for use in this scope.
    std::optional<WindowInfo>* result;  // Declares result for use in this scope.
  } context{&targetWindowTitle_, &gameWindow_};  // Declares context and initializes it with &targetWindowTitle_, &gameWindow_.

  gameWindow_.reset();  // Calls reset on gameWindow_.

  EnumWindows(  // Continues the surrounding declaration or control-flow expression.
      [](HWND hwnd, LPARAM lparam) -> BOOL {  // Starts a multi-line initializer or scope for [](HWND hwnd, LPARAM lparam) -> BOOL.
        auto* context = reinterpret_cast<SearchContext*>(lparam);  // Sets auto* context to reinterpret_cast<SearchContext*>(lparam).
        if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) {  // Guards the following work behind the condition !IsWindowVisible(hwnd) || IsIconic(hwnd).
          return TRUE;  // Returns TRUE to the caller.
        }  // Ends the current code block.

        std::array<char, 512> title{};  // Declares title with value initialization.
        GetWindowTextA(hwnd, title.data(), static_cast<int>(title.size()));  // Invokes GetWindowTextA with the supplied arguments.
        const std::string windowTitle(title.data());  // Executes const std::string windowTitle(title.data()).
        if (windowTitle.find(*context->title) == std::string::npos) {  // Guards the following work behind the condition windowTitle.find(*context->title) == std::string::npos.
          return TRUE;  // Returns TRUE to the caller.
        }  // Ends the current code block.

        RECT rect{};  // Declares rect with value initialization.
        GetWindowRect(hwnd, &rect);  // Invokes GetWindowRect with the supplied arguments.

        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);  // Sets HMONITOR monitor to MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST).
        MONITORINFO monitorInfo{};  // Declares monitorInfo with value initialization.
        monitorInfo.cbSize = sizeof(MONITORINFO);  // Sets monitorInfo.cbSize to sizeof(MONITORINFO).
        GetMonitorInfo(monitor, &monitorInfo);  // Invokes GetMonitorInfo with the supplied arguments.

        DisplayInfo display{  // Starts a multi-line initializer or scope for DisplayInfo display.
            "active",  // Supplies "active" to the surrounding call or initializer.
            Rect{monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,  // Supplies Rect{monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top to the surrounding call or initializer.
                 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,  // Supplies monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left to the surrounding call or initializer.
                 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top},  // Finishes this initializer entry for the surrounding aggregate.
            1.0F,  // Supplies 1.0F to the surrounding call or initializer.
            false,  // Supplies false to the surrounding call or initializer.
        };  // Ends the current type, struct, or initializer declaration.

        *context->result = WindowInfo{  // Starts a multi-line initializer or scope for *context->result = WindowInfo.
            hwnd,  // Supplies hwnd to the surrounding call or initializer.
            windowTitle,  // Supplies windowTitle to the surrounding call or initializer.
            Rect{rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top},  // Finishes this initializer entry for the surrounding aggregate.
            display,  // Supplies display to the surrounding call or initializer.
        };  // Ends the current type, struct, or initializer declaration.
        return FALSE;  // Returns FALSE to the caller.
      },  // Supplies } to the surrounding call or initializer.
      reinterpret_cast<LPARAM>(&context));  // Executes reinterpret_cast<LPARAM>(&context)).
#else  // Selects this compile-time branch when earlier branches were not selected.
  gameWindow_.reset();  // Calls reset on gameWindow_.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

}  // Ends the current code block.
