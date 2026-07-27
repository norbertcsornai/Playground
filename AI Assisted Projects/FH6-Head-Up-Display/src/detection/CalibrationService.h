#pragma once  // Prevents this header from being included more than once.

#include <optional>  // Imports the optional standard library declarations used in this file.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class CalibrationService {  // Declares the CalibrationService class interface and members.
 public:  // Makes the following members part of the public API.
  CalibrationService();  // Invokes CalibrationService with the supplied arguments.

  [[nodiscard]] std::optional<Rect> autoLocateGearRegion(const Frame& frame) const;  // Declares autoLocateGearRegion and marks its return value as important.
  void setManualRegion(const Rect& region);  // Declares function setManualRegion for callers.
  [[nodiscard]] Rect getGearRegion() const;  // Declares getGearRegion and marks its return value as important.
  [[nodiscard]] Rect getGearRegion(const Frame& frame) const;  // Declares getGearRegion and marks its return value as important.
  [[nodiscard]] Rect getGearRegionForSize(int width, int height) const;  // Declares getGearRegionForSize and marks its return value as important.
  void resetToDefaults();  // Declares function resetToDefaults for callers.

 private:  // Makes the following members private implementation details.
  [[nodiscard]] Rect resolveDynamicRegion(int displayWidth, int displayHeight) const;  // Declares resolveDynamicRegion and marks its return value as important.

  Rect defaultRegion_{};  // Declares defaultRegion_ with value initialization.
  Rect currentRegion_{};  // Declares currentRegion_ with value initialization.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
