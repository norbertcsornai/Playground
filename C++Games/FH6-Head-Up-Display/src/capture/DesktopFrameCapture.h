#pragma once

#include <memory>
#include <optional>

#include "capture/IFrameCapture.h"

namespace fh6 {

class DesktopFrameCapture final : public IFrameCapture {
 public:
  explicit DesktopFrameCapture(int captureRateLimitFps = 30);
  ~DesktopFrameCapture() override;

  bool start(const DisplayInfo& display) override;
  [[nodiscard]] std::optional<Frame> captureFrame() override;
  void stop() override;
  [[nodiscard]] bool isAvailable() const override;
  void handleDisplayChanged(const DisplayInfo& display);
  void setCaptureRateLimit(int captureRateLimitFps);

 private:
  struct DxgiCaptureState;

  [[nodiscard]] bool initializeDxgiCapture();
  [[nodiscard]] std::optional<Frame> captureFrameWithDxgi(TimePoint timestamp);
  [[nodiscard]] std::optional<Frame> captureFrameWithGdi(TimePoint timestamp) const;

  DisplayInfo display_{};
  int captureRateLimitFps_{30};
  bool available_{false};
  TimePoint lastCapture_{};
  std::unique_ptr<DxgiCaptureState> dxgiState_{};
};

}  // namespace fh6
