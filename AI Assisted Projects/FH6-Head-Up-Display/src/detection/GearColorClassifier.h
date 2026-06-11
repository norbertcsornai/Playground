#pragma once  // Prevents this header from being included more than once.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.
#include "shared/Color.h"  // Imports project declarations from shared/Color.h.
#include "shared/GearTypes.h"  // Imports project declarations from shared/GearTypes.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class GearColorClassifier {  // Declares the GearColorClassifier class interface and members.
 public:  // Makes the following members part of the public API.
  GearColorClassifier() = default;  // Sets GearColorClassifier() to default.
  explicit GearColorClassifier(ColorThresholds thresholds);  // Declares the explicit GearColorClassifier constructor.

  [[nodiscard]] GearColorState classify(const FrameRegion& region) const;  // Declares classify and marks its return value as important.
  void updateThresholds(const ColorThresholds& thresholds);  // Declares function updateThresholds for callers.

 private:  // Makes the following members private implementation details.
  struct MatchStats {  // Declares the MatchStats value type and fields.
    int count{0};  // Declares count and initializes it with 0.
    float ratio{0.0F};  // Declares ratio and initializes it with 0.0F.
  };  // Ends the current type, struct, or initializer declaration.

  struct WidgetMatchStats {  // Declares the WidgetMatchStats value type and fields.
    int ringCount{0};  // Declares ringCount and initializes it with 0.
    int innerCount{0};  // Declares innerCount and initializes it with 0.
    int ringPixels{0};  // Declares ringPixels and initializes it with 0.
    int innerPixels{0};  // Declares innerPixels and initializes it with 0.
  };  // Ends the current type, struct, or initializer declaration.

  struct WidgetColorStats {  // Declares the WidgetColorStats value type and fields.
    WidgetMatchStats red;  // Declares red for use in this scope.
    WidgetMatchStats white;  // Declares white for use in this scope.
  };  // Ends the current type, struct, or initializer declaration.

  [[nodiscard]] MatchStats calculateMatchStats(const FrameRegion& region,  // Supplies [[nodiscard]] MatchStats calculateMatchStats(const FrameRegion& region to the surrounding call or initializer.
                                               const ColorThreshold& target) const;  // Declares const for use in this scope.
  [[nodiscard]] WidgetColorStats calculateWidgetColorStats(const FrameRegion& region) const;  // Declares calculateWidgetColorStats and marks its return value as important.
  [[nodiscard]] bool hasEnoughGlyphPixels(const MatchStats& stats) const;  // Declares hasEnoughGlyphPixels and marks its return value as important.
  [[nodiscard]] bool hasRedGearAndRing(const WidgetMatchStats& stats, int side) const;  // Declares hasRedGearAndRing and marks its return value as important.
  [[nodiscard]] bool hasWhiteGearSignal(const WidgetMatchStats& stats, int side) const;  // Declares hasWhiteGearSignal and marks its return value as important.

  ColorThresholds thresholds_{};  // Declares thresholds_ with value initialization.
  float confidenceThreshold_{0.001F};  // Declares confidenceThreshold_ and initializes it with 0.001F.
  int minimumGlyphPixels_{12};  // Declares minimumGlyphPixels_ and initializes it with 12.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
