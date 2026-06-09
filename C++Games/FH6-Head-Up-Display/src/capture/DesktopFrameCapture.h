#pragma once  // codex-line-comment: documents this line.

#include <memory>  // codex-line-comment: documents this line.
#include <optional>  // codex-line-comment: documents this line.

#include "capture/IFrameCapture.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class DesktopFrameCapture final : public IFrameCapture {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  explicit DesktopFrameCapture(int captureRateLimitFps = 30);  // codex-line-comment: documents this line.
  ~DesktopFrameCapture() override;  // codex-line-comment: documents this line.

  bool start(const DisplayInfo& display) override;  // codex-line-comment: documents this line.
  [[nodiscard]] std::optional<Frame> captureFrame() override;  // codex-line-comment: documents this line.
  void stop() override;  // codex-line-comment: documents this line.
  [[nodiscard]] bool isAvailable() const override;  // codex-line-comment: documents this line.
  void handleDisplayChanged(const DisplayInfo& display);  // codex-line-comment: documents this line.
  void setCaptureRateLimit(int captureRateLimitFps);  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  struct DxgiCaptureState;  // codex-line-comment: documents this line.

  [[nodiscard]] bool initializeDxgiCapture();  // codex-line-comment: documents this line.
  [[nodiscard]] std::optional<Frame> captureFrameWithDxgi(TimePoint timestamp);  // codex-line-comment: documents this line.
  [[nodiscard]] std::optional<Frame> captureFrameWithGdi(TimePoint timestamp) const;  // codex-line-comment: documents this line.

  DisplayInfo display_{};  // codex-line-comment: documents this line.
  int captureRateLimitFps_{30};  // codex-line-comment: documents this line.
  bool available_{false};  // codex-line-comment: documents this line.
  TimePoint lastCapture_{};  // codex-line-comment: documents this line.
  std::unique_ptr<DxgiCaptureState> dxgiState_{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
