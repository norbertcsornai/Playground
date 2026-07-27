#pragma once  // Prevents this header from being included more than once.

#include <optional>  // Imports the optional standard library declarations used in this file.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.
#include "shared/DisplayInfo.h"  // Imports project declarations from shared/DisplayInfo.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class IFrameCapture {  // Declares the IFrameCapture class interface and members.
 public:  // Makes the following members part of the public API.
  virtual ~IFrameCapture() = default;  // Sets virtual ~IFrameCapture() to default.

  virtual bool start(const DisplayInfo& display) = 0;  // Sets virtual bool start(const DisplayInfo& display) to 0.
  [[nodiscard]] virtual std::optional<Frame> captureFrame() = 0;  // Sets [[nodiscard]] virtual std::optional<Frame> captureFrame() to 0.
  virtual void stop() = 0;  // Sets virtual void stop() to 0.
  [[nodiscard]] virtual bool isAvailable() const = 0;  // Sets [[nodiscard]] virtual bool isAvailable() const to 0.

  // Restricts capture to the given display-coordinate rect so implementations can skip reading
  // pixels the caller will never inspect. An empty rect means capture the whole display.
  // Implementations may ignore this; callers must not assume the returned Frame is cropped.
  virtual void setRegionOfInterest(const Rect& region) { (void)region; }  // Provides a no-op default for implementations that always capture full frames.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
