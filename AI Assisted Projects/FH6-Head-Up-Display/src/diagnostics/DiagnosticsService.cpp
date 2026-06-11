#include "diagnostics/DiagnosticsService.h"  // Imports project declarations from diagnostics/DiagnosticsService.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

DiagnosticsService::DiagnosticsService(std::ostream& output) : output_(&output) {}  // Finishes this initializer entry for the surrounding aggregate.

void DiagnosticsService::setEnabled(bool enabled) {  // Implements DiagnosticsService::setEnabled.
  enabled_ = enabled;  // Sets enabled_ to enabled.
}  // Ends the current code block.

void DiagnosticsService::reportGameDetected(bool detected) {  // Implements DiagnosticsService::reportGameDetected.
  log(detected ? "Game detected" : "Game not detected");  // Invokes log with the supplied arguments.
}  // Ends the current code block.

void DiagnosticsService::reportFrameStatus(FrameStatus status) {  // Implements DiagnosticsService::reportFrameStatus.
  log("Frame status: " + std::to_string(static_cast<int>(status)));  // Invokes log with the supplied arguments.
}  // Ends the current code block.

void DiagnosticsService::reportDetection(const GearDetectionResult& result) {  // Implements DiagnosticsService::reportDetection.
  log("Detection confidence: " + std::to_string(result.confidence));  // Invokes log with the supplied arguments.
}  // Ends the current code block.

void DiagnosticsService::reportOverlayStatus(OverlayStatus status) {  // Implements DiagnosticsService::reportOverlayStatus.
  log("Overlay status: " + std::to_string(static_cast<int>(status)));  // Invokes log with the supplied arguments.
}  // Ends the current code block.

void DiagnosticsService::log(const std::string& message) {  // Implements DiagnosticsService::log.
  if (enabled_ && output_ != nullptr) {  // Guards the following work behind the condition enabled_ && output_ != nullptr.
    *output_ << "[fh6-hud] " << message << '\n';  // Executes *output_ << "[fh6-hud] " << message << '\n'.
  }  // Ends the current code block.
}  // Ends the current code block.

}  // Ends the current code block.
