#pragma once  // Prevents this header from being included more than once.

#include <iostream>  // Imports the iostream standard library declarations used in this file.
#include <string>  // Imports the string standard library declarations used in this file.

#include "detection/GearDetectionResult.h"  // Imports project declarations from detection/GearDetectionResult.h.
#include "shared/Status.h"  // Imports project declarations from shared/Status.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class DiagnosticsService {  // Declares the DiagnosticsService class interface and members.
 public:  // Makes the following members part of the public API.
  explicit DiagnosticsService(std::ostream& output = std::clog);  // Declares the explicit DiagnosticsService constructor.

  void setEnabled(bool enabled);  // Declares function setEnabled for callers.
  void reportGameDetected(bool detected);  // Declares function reportGameDetected for callers.
  void reportFrameStatus(FrameStatus status);  // Declares function reportFrameStatus for callers.
  void reportDetection(const GearDetectionResult& result);  // Declares function reportDetection for callers.
  void reportOverlayStatus(OverlayStatus status);  // Declares function reportOverlayStatus for callers.
  void log(const std::string& message);  // Declares function log for callers.

 private:  // Makes the following members private implementation details.
  bool enabled_{false};  // Declares enabled_ and initializes it with false.
  std::ostream* output_{nullptr};  // Declares output_ and initializes it with nullptr.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
