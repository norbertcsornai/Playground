#pragma once  // codex-line-comment: documents this line.

#include <iostream>  // codex-line-comment: documents this line.
#include <string>  // codex-line-comment: documents this line.

#include "detection/GearDetectionResult.h"  // codex-line-comment: documents this line.
#include "shared/Status.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class DiagnosticsService {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  explicit DiagnosticsService(std::ostream& output = std::clog);  // codex-line-comment: documents this line.

  void setEnabled(bool enabled);  // codex-line-comment: documents this line.
  void reportGameDetected(bool detected);  // codex-line-comment: documents this line.
  void reportFrameStatus(FrameStatus status);  // codex-line-comment: documents this line.
  void reportDetection(const GearDetectionResult& result);  // codex-line-comment: documents this line.
  void reportOverlayStatus(OverlayStatus status);  // codex-line-comment: documents this line.
  void log(const std::string& message);  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  bool enabled_{false};  // codex-line-comment: documents this line.
  std::ostream* output_{nullptr};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
