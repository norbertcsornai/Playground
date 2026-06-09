#pragma once

#include <iostream>
#include <string>

#include "detection/GearDetectionResult.h"
#include "shared/Status.h"

namespace fh6 {

class DiagnosticsService {
 public:
  explicit DiagnosticsService(std::ostream& output = std::clog);

  void setEnabled(bool enabled);
  void reportGameDetected(bool detected);
  void reportFrameStatus(FrameStatus status);
  void reportDetection(const GearDetectionResult& result);
  void reportOverlayStatus(OverlayStatus status);
  void log(const std::string& message);

 private:
  bool enabled_{false};
  std::ostream* output_{nullptr};
};

}  // namespace fh6
