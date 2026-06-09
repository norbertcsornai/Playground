#pragma once  // codex-line-comment: documents this line.

#include <optional>  // codex-line-comment: documents this line.

#include "capture/Frame.h"  // codex-line-comment: documents this line.
#include "shared/DisplayInfo.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class IFrameCapture {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  virtual ~IFrameCapture() = default;  // codex-line-comment: documents this line.

  virtual bool start(const DisplayInfo& display) = 0;  // codex-line-comment: documents this line.
  [[nodiscard]] virtual std::optional<Frame> captureFrame() = 0;  // codex-line-comment: documents this line.
  virtual void stop() = 0;  // codex-line-comment: documents this line.
  [[nodiscard]] virtual bool isAvailable() const = 0;  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
