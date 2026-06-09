#include "diagnostics/DiagnosticsService.h"

namespace fh6 {

DiagnosticsService::DiagnosticsService(std::ostream& output) : output_(&output) {}

void DiagnosticsService::setEnabled(bool enabled) {
  enabled_ = enabled;
}

void DiagnosticsService::reportGameDetected(bool detected) {
  log(detected ? "Game detected" : "Game not detected");
}

void DiagnosticsService::reportFrameStatus(FrameStatus status) {
  log("Frame status: " + std::to_string(static_cast<int>(status)));
}

void DiagnosticsService::reportDetection(const GearDetectionResult& result) {
  log("Detection confidence: " + std::to_string(result.confidence));
}

void DiagnosticsService::reportOverlayStatus(OverlayStatus status) {
  log("Overlay status: " + std::to_string(static_cast<int>(status)));
}

void DiagnosticsService::log(const std::string& message) {
  if (enabled_ && output_ != nullptr) {
    *output_ << "[fh6-hud] " << message << '\n';
  }
}

}  // namespace fh6
