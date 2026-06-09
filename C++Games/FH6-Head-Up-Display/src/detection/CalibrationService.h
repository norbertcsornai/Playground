#pragma once  // codex-line-comment: documents this line.

#include <optional>  // codex-line-comment: documents this line.

#include "capture/Frame.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class CalibrationService {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  CalibrationService();  // codex-line-comment: documents this line.

  [[nodiscard]] std::optional<Rect> autoLocateGearRegion(const Frame& frame) const;  // codex-line-comment: documents this line.
  void setManualRegion(const Rect& region);  // codex-line-comment: documents this line.
  [[nodiscard]] Rect getGearRegion() const;  // codex-line-comment: documents this line.
  [[nodiscard]] Rect getGearRegion(const Frame& frame) const;  // codex-line-comment: documents this line.
  void resetToDefaults();  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  [[nodiscard]] Rect resolveDynamicRegion(const Frame& frame) const;  // codex-line-comment: documents this line.

  Rect defaultRegion_{};  // codex-line-comment: documents this line.
  Rect currentRegion_{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
