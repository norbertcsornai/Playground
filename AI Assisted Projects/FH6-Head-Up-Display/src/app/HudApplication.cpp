#include "app/HudApplication.h"  // Imports project declarations from app/HudApplication.h.

#include <chrono>  // Imports the chrono standard library declarations used in this file.
#include <csignal>  // Imports the csignal standard library declarations used in this file.
#include <memory>  // Imports the memory standard library declarations used in this file.
#include <thread>  // Imports the thread standard library declarations used in this file.

#include "capture/DesktopFrameCapture.h"  // Imports project declarations from capture/DesktopFrameCapture.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

HudApplication::HudApplication()  // Begins the multi-line constructor definition for HudApplication.
    : frameCapture_(std::make_unique<DesktopFrameCapture>()), shiftAlertController_(config_.arrowDuration) {}  // Initializes constructor members with frameCapture_(std::make_unique<DesktopFrameCapture>()), shiftAlertController_(config_.a....

HudApplication::HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore)  // Begins the multi-line constructor definition for HudApplication.
    : configStore_(std::move(configStore)), frameCapture_(std::move(frameCapture)) {}  // Initializes constructor members with configStore_(std::move(configStore)), frameCapture_(std::move(frameCapture)).

bool HudApplication::initialize() {  // Implements HudApplication::initialize.
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
  while (running_) {  // Repeats while running_ remains true.
    processFrame();  // Invokes processFrame with the supplied arguments.
    std::this_thread::sleep_for(std::chrono::milliseconds(5));  // Executes std::this_thread::sleep_for(std::chrono::milliseconds(5)).
  }  // Ends the current code block.
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
