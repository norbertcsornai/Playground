#include "app/HudApplication.h"  // Imports project declarations from app/HudApplication.h.

#include <chrono>  // Imports the chrono standard library declarations used in this file.
#include <csignal>  // Imports the csignal standard library declarations used in this file.
#include <memory>  // Imports the memory standard library declarations used in this file.
#include <thread>  // Imports the thread standard library declarations used in this file.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
#include <windows.h>  // Imports the windows.h standard library declarations used in this file.
#include <mmsystem.h>  // Imports timeBeginPeriod/timeEndPeriod for requesting high-resolution sleep timing.
#endif  // Ends the compile-time selection block.

#include "capture/DesktopFrameCapture.h"  // Imports project declarations from capture/DesktopFrameCapture.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

namespace {  // Starts a file-local helper namespace.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
// Without this, Windows virtualizes monitor metrics for a scaled display: on a 2560x1440 monitor at
// 125% scaling GetMonitorInfo reports 2048x1152, while DXGI desktop duplication still hands back the
// true 2560x1440 surface. Region math derived from the reported size then addresses the wrong part
// of the captured texture. Declaring per-monitor awareness makes both report physical pixels, and
// also keeps the overlay positioned in the same space the capture uses.
void enablePerMonitorDpiAwareness() {  // Begins function enablePerMonitorDpiAwareness.
  using SetContextFn = BOOL(WINAPI*)(HANDLE);  // Aliases SetContextFn to the SetProcessDpiAwarenessContext signature.
  if (HMODULE user32 = GetModuleHandleA("user32.dll")) {  // Guards the following work behind loading user32.
    // Resolved dynamically because SetProcessDpiAwarenessContext needs Windows 10 1703 or newer.
    if (auto setContext = reinterpret_cast<SetContextFn>(  // Sets auto setContext to the resolved entry point.
            reinterpret_cast<void*>(GetProcAddress(user32, "SetProcessDpiAwarenessContext")))) {  // Executes reinterpret_cast<void*>(GetProcAddress(user32, "SetProcessDpiAwarenessContext")))).
      if (setContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {  // Guards the following work behind a successful call.
        return;  // Leaves this function without a return value.
      }  // Ends the current code block.
    }  // Ends the current code block.
  }  // Ends the current code block.

  SetProcessDPIAware();  // Falls back to the older system-wide DPI awareness entry point.
}  // Ends the current code block.
#endif  // Ends the compile-time selection block.

}  // Ends the current code block.

HudApplication::HudApplication()  // Begins the multi-line constructor definition for HudApplication.
    : frameCapture_(std::make_unique<DesktopFrameCapture>()), shiftAlertController_(config_.arrowDuration) {}  // Initializes constructor members with frameCapture_(std::make_unique<DesktopFrameCapture>()), shiftAlertController_(config_.a....

HudApplication::HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore)  // Begins the multi-line constructor definition for HudApplication.
    : configStore_(std::move(configStore)), frameCapture_(std::move(frameCapture)) {}  // Initializes constructor members with configStore_(std::move(configStore)), frameCapture_(std::move(frameCapture)).

bool HudApplication::initialize() {  // Implements HudApplication::initialize.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  enablePerMonitorDpiAwareness();  // Aligns reported monitor metrics with the captured surface before anything queries them.
#endif  // Ends the compile-time selection block.

  config_ = configStore_.load();  // Sets config_ to configStore_.load().
  if (auto* desktopCapture = dynamic_cast<DesktopFrameCapture*>(frameCapture_.get())) {  // Guards the following work behind the condition auto* desktopCapture = dynamic_cast<DesktopFrameCapture*>(frameCapture_.get()).
    desktopCapture->setCaptureRateLimit(config_.captureRateLimitFps);  // Calls setCaptureRateLimit through desktopCapture.
  }  // Ends the current code block.
  gearDetector_.setRegion(config_.gearRegion);  // Calls setRegion on gearDetector_.
  gearColorClassifier_.updateThresholds(config_.colorThresholds);  // Calls updateThresholds on gearColorClassifier_.
  shiftAlertController_ = ShiftAlertController(config_.arrowDuration);  // Sets shiftAlertController_ to ShiftAlertController(config_.arrowDuration).
  overlayWindow_.setArrowSize(config_.arrowSize);  // Calls setArrowSize on overlayWindow_.
  diagnostics_.setEnabled(config_.diagnosticsEnabled);  // Calls setEnabled on diagnostics_.
  running_ = true;  // Sets running_ to true.
  return true;  // Returns true to the caller.
}  // Ends the current code block.

void HudApplication::run() {  // Implements HudApplication::run.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  // Windows' default system timer resolution is ~15.6ms, which silently rounds a 1ms sleep_for
  // up to that granularity and caps this loop (and therefore trigger latency) at ~64Hz no matter
  // how high the capture rate limit is set. Requesting 1ms resolution for the loop's lifetime
  // makes the sleep below actually behave like 1ms.
  timeBeginPeriod(1);  // Requests 1ms system timer resolution for this process.
#endif  // Ends the compile-time selection block.

  while (running_) {  // Repeats while running_ remains true.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
    // The overlay window lives on this thread, so its messages have to be drained here. Without a
    // pump the paint handler never runs and Windows eventually treats the window as unresponsive.
    MSG message;  // Declares message for use in this scope.
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {  // Repeats while a queued message is available.
      TranslateMessage(&message);  // Invokes TranslateMessage with the supplied arguments.
      DispatchMessageA(&message);  // Invokes DispatchMessageA with the supplied arguments.
    }  // Ends the current code block.
#endif  // Ends the compile-time selection block.

    processFrame();  // Invokes processFrame with the supplied arguments.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Executes std::this_thread::sleep_for(std::chrono::milliseconds(1)).
  }  // Ends the current code block.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  timeEndPeriod(1);  // Releases the 1ms system timer resolution request.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

void HudApplication::requestStop() {  // Implements HudApplication::requestStop.
  running_ = false;  // Sets running_ to false.
}  // Ends the current code block.

void HudApplication::shutdown() {  // Implements HudApplication::shutdown.
  running_ = false;  // Sets running_ to false.
  if (frameCapture_) {  // Guards the following work behind the condition frameCapture_.
    frameCapture_->stop();  // Calls stop through frameCapture_.
  }  // Ends the current code block.
  overlayWindow_.destroy();  // Calls destroy on overlayWindow_.
}  // Ends the current code block.

void HudApplication::processFrame() {  // Implements HudApplication::processFrame.
  // refresh() enumerates every top-level window and reads each one's title, which is far too
  // expensive to repeat on every pass of a millisecond loop -- doing so measurably steals time from
  // the foreground game. The game's window does not change identity at that rate, so rescan only
  // periodically, or immediately while no window is known yet so startup still latches on quickly.
  // The cheap per-pass checks below reuse the cached handle, so alt-tab is still noticed at once.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().
  if (!gameWindowTracker_.isGameRunning() || now - lastWindowScan_ >= kWindowScanInterval) {  // Guards the rescan behind a missing window or an elapsed interval.
    gameWindowTracker_.refresh();  // Invokes refresh with the supplied arguments.
    lastWindowScan_ = now;  // Sets lastWindowScan_ to now.
  }  // Ends the current code block.

  if (!gameWindowTracker_.isGameVisible()) {  // Guards the following work behind the condition !gameWindowTracker_.isGameVisible().
    diagnostics_.reportGameDetected(false);  // Calls reportGameDetected on diagnostics_.
    enterIdleMode();  // Invokes enterIdleMode with the supplied arguments.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  diagnostics_.reportGameDetected(true);  // Calls reportGameDetected on diagnostics_.
  ensureCaptureForActiveDisplay();  // Invokes ensureCaptureForActiveDisplay with the supplied arguments.

  if (!frameCapture_ || !frameCapture_->isAvailable()) {  // Guards the following work behind the condition !frameCapture_ || !frameCapture_->isAvailable().
    diagnostics_.reportFrameStatus(FrameStatus::Unavailable);  // Calls reportFrameStatus on diagnostics_.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  auto frame = frameCapture_->captureFrame();  // Sets auto frame to frameCapture_->captureFrame().
  if (!frame) {  // Guards the following work behind the condition !frame.
    diagnostics_.reportFrameStatus(FrameStatus::Idle);  // Calls reportFrameStatus on diagnostics_.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  lastFrameTime_ = frame->timestamp();  // Sets lastFrameTime_ to frame->timestamp().
  diagnostics_.reportFrameStatus(FrameStatus::Capturing);  // Calls reportFrameStatus on diagnostics_.

  auto result = gearDetector_.detectGear(*frame, gearColorClassifier_);  // Sets auto result to gearDetector_.detectGear(*frame, gearColorClassifier_).
  diagnostics_.reportDetection(result);  // Calls reportDetection on diagnostics_.

  const auto alert = shiftAlertController_.update(result, Clock::now());  // Sets const auto alert to shiftAlertController_.update(result, Clock::now()).
  if (alert.active) {  // Guards the following work behind the condition alert.active.
    overlayWindow_.centerOnDisplay(gameWindowTracker_.getActiveDisplay());  // Calls centerOnDisplay on overlayWindow_.
    overlayWindow_.showArrow();  // Calls showArrow on overlayWindow_.
    diagnostics_.reportOverlayStatus(OverlayStatus::Visible);  // Calls reportOverlayStatus on diagnostics_.
  } else {  // Handles the fallback case for the preceding condition.
    overlayWindow_.hideArrow();  // Calls hideArrow on overlayWindow_.
    diagnostics_.reportOverlayStatus(OverlayStatus::Hidden);  // Calls reportOverlayStatus on diagnostics_.
  }  // Ends the current code block.
}  // Ends the current code block.

void HudApplication::enterIdleMode() {  // Implements HudApplication::enterIdleMode.
  if (frameCapture_) {  // Guards the following work behind the condition frameCapture_.
    frameCapture_->stop();  // Calls stop through frameCapture_.
  }  // Ends the current code block.
  shiftAlertController_.reset();  // Calls reset on shiftAlertController_.
  overlayWindow_.hideArrow();  // Calls hideArrow on overlayWindow_.
}  // Ends the current code block.

void HudApplication::ensureCaptureForActiveDisplay() {  // Implements HudApplication::ensureCaptureForActiveDisplay.
  const auto display = gameWindowTracker_.getActiveDisplay();  // Sets const auto display to gameWindowTracker_.getActiveDisplay().
  if (display.bounds.empty()) {  // Guards the following work behind the condition display.bounds.empty().
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  // Tell the capture layer to read only the HUD rect the detector will inspect. Converting a whole
  // screen's pixels every frame was the dominant per-frame cost; this scales it to the HUD instead.
  if (frameCapture_) {  // Guards the following work behind the condition frameCapture_.
    frameCapture_->setRegionOfInterest(  // Calls setRegionOfInterest through frameCapture_.
        gearDetector_.gearRegionForDisplay(display.bounds.width, display.bounds.height));  // Supplies the HUD rect for this display size.
  }  // Ends the current code block.

  if (!overlayWindow_.isCreated()) {  // Guards the following work behind the condition !overlayWindow_.isCreated().
    if (!overlayWindow_.create(display)) {  // Guards the following work behind the condition !overlayWindow_.create(display).
      diagnostics_.reportOverlayStatus(OverlayStatus::Failed);  // Calls reportOverlayStatus on diagnostics_.
    }  // Ends the current code block.
  }  // Ends the current code block.

  if (frameCapture_ && !frameCapture_->isAvailable()) {  // Guards the following work behind the condition frameCapture_ && !frameCapture_->isAvailable().
    if (!frameCapture_->start(display)) {  // Guards the following work behind the condition !frameCapture_->start(display).
      diagnostics_.reportFrameStatus(FrameStatus::Failed);  // Calls reportFrameStatus on diagnostics_.
    }  // Ends the current code block.
  }  // Ends the current code block.
}  // Ends the current code block.

}  // Ends the current code block.
