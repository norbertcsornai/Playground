#include "app/HudApplication.h"  // codex-line-comment: documents this line.

#include <chrono>  // codex-line-comment: documents this line.
#include <csignal>  // codex-line-comment: documents this line.
#include <memory>  // codex-line-comment: documents this line.
#include <thread>  // codex-line-comment: documents this line.

#include "capture/DesktopFrameCapture.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

HudApplication::HudApplication()  // codex-line-comment: documents this line.
    : frameCapture_(std::make_unique<DesktopFrameCapture>()), shiftAlertController_(config_.arrowDuration) {}  // codex-line-comment: documents this line.

HudApplication::HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore)  // codex-line-comment: documents this line.
    : configStore_(std::move(configStore)), frameCapture_(std::move(frameCapture)) {}  // codex-line-comment: documents this line.

bool HudApplication::initialize() {  // codex-line-comment: documents this line.
  config_ = configStore_.load();  // codex-line-comment: documents this line.
  if (auto* desktopCapture = dynamic_cast<DesktopFrameCapture*>(frameCapture_.get())) {  // codex-line-comment: documents this line.
    desktopCapture->setCaptureRateLimit(config_.captureRateLimitFps);  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  gearDetector_.setRegion(config_.gearRegion);  // codex-line-comment: documents this line.
  gearColorClassifier_.updateThresholds(config_.colorThresholds);  // codex-line-comment: documents this line.
  shiftAlertController_ = ShiftAlertController(config_.arrowDuration);  // codex-line-comment: documents this line.
  overlayWindow_.setArrowSize(config_.arrowSize);  // codex-line-comment: documents this line.
  diagnostics_.setEnabled(config_.diagnosticsEnabled);  // codex-line-comment: documents this line.
  running_ = true;  // codex-line-comment: documents this line.
  return true;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void HudApplication::run() {  // codex-line-comment: documents this line.
  while (running_) {  // codex-line-comment: documents this line.
    processFrame();  // codex-line-comment: documents this line.
    std::this_thread::sleep_for(std::chrono::milliseconds(5));  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void HudApplication::requestStop() {  // codex-line-comment: documents this line.
  running_ = false;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void HudApplication::shutdown() {  // codex-line-comment: documents this line.
  running_ = false;  // codex-line-comment: documents this line.
  if (frameCapture_) {  // codex-line-comment: documents this line.
    frameCapture_->stop();  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  overlayWindow_.destroy();  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void HudApplication::processFrame() {  // codex-line-comment: documents this line.
  if (!gameWindowTracker_.isGameVisible()) {  // codex-line-comment: documents this line.
    diagnostics_.reportGameDetected(false);  // codex-line-comment: documents this line.
    enterIdleMode();  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  diagnostics_.reportGameDetected(true);  // codex-line-comment: documents this line.
  ensureCaptureForActiveDisplay();  // codex-line-comment: documents this line.

  if (!frameCapture_ || !frameCapture_->isAvailable()) {  // codex-line-comment: documents this line.
    diagnostics_.reportFrameStatus(FrameStatus::Unavailable);  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  auto frame = frameCapture_->captureFrame();  // codex-line-comment: documents this line.
  if (!frame) {  // codex-line-comment: documents this line.
    diagnostics_.reportFrameStatus(FrameStatus::Idle);  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  lastFrameTime_ = frame->timestamp();  // codex-line-comment: documents this line.
  diagnostics_.reportFrameStatus(FrameStatus::Capturing);  // codex-line-comment: documents this line.

  auto result = gearDetector_.detectGear(*frame, gearColorClassifier_);  // codex-line-comment: documents this line.
  diagnostics_.reportDetection(result);  // codex-line-comment: documents this line.

  const auto alert = shiftAlertController_.update(result, Clock::now());  // codex-line-comment: documents this line.
  if (alert.active) {  // codex-line-comment: documents this line.
    overlayWindow_.centerOnDisplay(gameWindowTracker_.getActiveDisplay());  // codex-line-comment: documents this line.
    overlayWindow_.showArrow();  // codex-line-comment: documents this line.
    diagnostics_.reportOverlayStatus(OverlayStatus::Visible);  // codex-line-comment: documents this line.
  } else {  // codex-line-comment: documents this line.
    overlayWindow_.hideArrow();  // codex-line-comment: documents this line.
    diagnostics_.reportOverlayStatus(OverlayStatus::Hidden);  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void HudApplication::enterIdleMode() {  // codex-line-comment: documents this line.
  if (frameCapture_) {  // codex-line-comment: documents this line.
    frameCapture_->stop();  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  shiftAlertController_.reset();  // codex-line-comment: documents this line.
  overlayWindow_.hideArrow();  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void HudApplication::ensureCaptureForActiveDisplay() {  // codex-line-comment: documents this line.
  const auto display = gameWindowTracker_.getActiveDisplay();  // codex-line-comment: documents this line.
  if (display.bounds.empty()) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (!overlayWindow_.isCreated()) {  // codex-line-comment: documents this line.
    if (!overlayWindow_.create(display)) {  // codex-line-comment: documents this line.
      diagnostics_.reportOverlayStatus(OverlayStatus::Failed);  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (frameCapture_ && !frameCapture_->isAvailable()) {  // codex-line-comment: documents this line.
    if (!frameCapture_->start(display)) {  // codex-line-comment: documents this line.
      diagnostics_.reportFrameStatus(FrameStatus::Failed);  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
