#include "diagnostics/DiagnosticsService.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

DiagnosticsService::DiagnosticsService(std::ostream& output) : output_(&output) {}  // codex-line-comment: documents this line.

void DiagnosticsService::setEnabled(bool enabled) {  // codex-line-comment: documents this line.
  enabled_ = enabled;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DiagnosticsService::reportGameDetected(bool detected) {  // codex-line-comment: documents this line.
  log(detected ? "Game detected" : "Game not detected");  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DiagnosticsService::reportFrameStatus(FrameStatus status) {  // codex-line-comment: documents this line.
  log("Frame status: " + std::to_string(static_cast<int>(status)));  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DiagnosticsService::reportDetection(const GearDetectionResult& result) {  // codex-line-comment: documents this line.
  log("Detection confidence: " + std::to_string(result.confidence));  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DiagnosticsService::reportOverlayStatus(OverlayStatus status) {  // codex-line-comment: documents this line.
  log("Overlay status: " + std::to_string(static_cast<int>(status)));  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DiagnosticsService::log(const std::string& message) {  // codex-line-comment: documents this line.
  if (enabled_ && output_ != nullptr) {  // codex-line-comment: documents this line.
    *output_ << "[fh6-hud] " << message << '\n';  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
