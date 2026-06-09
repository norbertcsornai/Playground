#pragma once

#include <optional>

#include "capture/Frame.h"
#include "shared/DisplayInfo.h"

namespace fh6 {

class IFrameCapture {
 public:
  virtual ~IFrameCapture() = default;

  virtual bool start(const DisplayInfo& display) = 0;
  [[nodiscard]] virtual std::optional<Frame> captureFrame() = 0;
  virtual void stop() = 0;
  [[nodiscard]] virtual bool isAvailable() const = 0;
};

}  // namespace fh6
