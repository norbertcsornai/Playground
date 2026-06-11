#pragma once  // Prevents this header from being included more than once.

#include <memory>  // Imports the memory standard library declarations used in this file.
#include <optional>  // Imports the optional standard library declarations used in this file.

#include "capture/IFrameCapture.h"  // Imports project declarations from capture/IFrameCapture.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class DesktopFrameCapture final : public IFrameCapture {  // Declares the DesktopFrameCapture class interface and members.
 public:  // Makes the following members part of the public API.
  explicit DesktopFrameCapture(int captureRateLimitFps = 30);  // Declares the explicit DesktopFrameCapture constructor.
  ~DesktopFrameCapture() override;  // Declares override for use in this scope.

  bool start(const DisplayInfo& display) override;  // Declares override for use in this scope.
  [[nodiscard]] std::optional<Frame> captureFrame() override;  // Declares override for use in this scope.
  void stop() override;  // Declares override for use in this scope.
  [[nodiscard]] bool isAvailable() const override;  // Declares override for use in this scope.
  void handleDisplayChanged(const DisplayInfo& display);  // Declares function handleDisplayChanged for callers.
  void setCaptureRateLimit(int captureRateLimitFps);  // Declares function setCaptureRateLimit for callers.

 private:  // Makes the following members private implementation details.
  struct DxgiCaptureState;  // Declares the DxgiCaptureState value type and fields.

  [[nodiscard]] bool initializeDxgiCapture();  // Declares initializeDxgiCapture and marks its return value as important.
  [[nodiscard]] std::optional<Frame> captureFrameWithDxgi(TimePoint timestamp);  // Declares captureFrameWithDxgi and marks its return value as important.
  [[nodiscard]] std::optional<Frame> captureFrameWithGdi(TimePoint timestamp) const;  // Declares captureFrameWithGdi and marks its return value as important.

  DisplayInfo display_{};  // Declares display_ with value initialization.
  int captureRateLimitFps_{30};  // Declares captureRateLimitFps_ and initializes it with 30.
  bool available_{false};  // Declares available_ and initializes it with false.
  TimePoint lastCapture_{};  // Declares lastCapture_ with value initialization.
  std::unique_ptr<DxgiCaptureState> dxgiState_{};  // Declares dxgiState_ with value initialization.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
