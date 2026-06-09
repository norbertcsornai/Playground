#include "app/HudApplication.h"

#include <chrono>
#include <csignal>
#include <memory>
#include <thread>

#include "capture/DesktopFrameCapture.h"

namespace fh6 {

HudApplication::HudApplication()
    : frameCapture_(std::make_unique<DesktopFrameCapture>()), shiftAlertController_(config_.arrowDuration) {}

HudApplication::HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore)
    : configStore_(std::move(configStore)), frameCapture_(std::move(frameCapture)) {}

bool HudApplication::initialize() {
  config_ = configStore_.load();
  if (auto* desktopCapture = dynamic_cast<DesktopFrameCapture*>(frameCapture_.get())) {
    desktopCapture->setCaptureRateLimit(config_.captureRateLimitFps);
  }
  gearDetector_.setRegion(config_.gearRegion);
  gearColorClassifier_.updateThresholds(config_.colorThresholds);
  shiftAlertController_ = ShiftAlertController(config_.arrowDuration);
  overlayWindow_.setArrowSize(config_.arrowSize);
  diagnostics_.setEnabled(config_.diagnosticsEnabled);
  running_ = true;
  return true;
}

void HudApplication::run() {
  while (running_) {
    processFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void HudApplication::requestStop() {
  running_ = false;
}

void HudApplication::shutdown() {
  running_ = false;
  if (frameCapture_) {
    frameCapture_->stop();
  }
  overlayWindow_.destroy();
}

void HudApplication::processFrame() {
  if (!gameWindowTracker_.isGameVisible()) {
    diagnostics_.reportGameDetected(false);
    enterIdleMode();
    return;
  }

  diagnostics_.reportGameDetected(true);
  ensureCaptureForActiveDisplay();

  if (!frameCapture_ || !frameCapture_->isAvailable()) {
    diagnostics_.reportFrameStatus(FrameStatus::Unavailable);
    return;
  }

  auto frame = frameCapture_->captureFrame();
  if (!frame) {
    diagnostics_.reportFrameStatus(FrameStatus::Idle);
    return;
  }

  lastFrameTime_ = frame->timestamp();
  diagnostics_.reportFrameStatus(FrameStatus::Capturing);

  auto result = gearDetector_.detectGear(*frame, gearColorClassifier_);
  diagnostics_.reportDetection(result);

  const auto alert = shiftAlertController_.update(result, Clock::now());
  if (alert.active) {
    overlayWindow_.centerOnDisplay(gameWindowTracker_.getActiveDisplay());
    overlayWindow_.showArrow();
    diagnostics_.reportOverlayStatus(OverlayStatus::Visible);
  } else {
    overlayWindow_.hideArrow();
    diagnostics_.reportOverlayStatus(OverlayStatus::Hidden);
  }
}

void HudApplication::enterIdleMode() {
  if (frameCapture_) {
    frameCapture_->stop();
  }
  shiftAlertController_.reset();
  overlayWindow_.hideArrow();
}

void HudApplication::ensureCaptureForActiveDisplay() {
  const auto display = gameWindowTracker_.getActiveDisplay();
  if (display.bounds.empty()) {
    return;
  }

  if (!overlayWindow_.isCreated()) {
    if (!overlayWindow_.create(display)) {
      diagnostics_.reportOverlayStatus(OverlayStatus::Failed);
    }
  }

  if (frameCapture_ && !frameCapture_->isAvailable()) {
    if (!frameCapture_->start(display)) {
      diagnostics_.reportFrameStatus(FrameStatus::Failed);
    }
  }
}

}  // namespace fh6
