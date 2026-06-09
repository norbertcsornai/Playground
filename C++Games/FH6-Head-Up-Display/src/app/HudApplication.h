#pragma once  // codex-line-comment: documents this line.

#include <atomic>  // codex-line-comment: documents this line.
#include <memory>  // codex-line-comment: documents this line.

#include "alert/ShiftAlertController.h"  // codex-line-comment: documents this line.
#include "capture/IFrameCapture.h"  // codex-line-comment: documents this line.
#include "config/AppConfig.h"  // codex-line-comment: documents this line.
#include "config/ConfigStore.h"  // codex-line-comment: documents this line.
#include "detection/GearColorClassifier.h"  // codex-line-comment: documents this line.
#include "detection/GearDetector.h"  // codex-line-comment: documents this line.
#include "diagnostics/DiagnosticsService.h"  // codex-line-comment: documents this line.
#include "overlay/OverlayWindow.h"  // codex-line-comment: documents this line.
#include "platform/GameWindowTracker.h"  // codex-line-comment: documents this line.
#include "shared/Time.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class HudApplication {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  HudApplication();  // codex-line-comment: documents this line.
  HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore);  // codex-line-comment: documents this line.

  bool initialize();  // codex-line-comment: documents this line.
  void run();  // codex-line-comment: documents this line.
  void requestStop();  // codex-line-comment: documents this line.
  void shutdown();  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  void processFrame();  // codex-line-comment: documents this line.
  void enterIdleMode();  // codex-line-comment: documents this line.
  void ensureCaptureForActiveDisplay();  // codex-line-comment: documents this line.

  std::atomic_bool running_{false};  // codex-line-comment: documents this line.
  TimePoint lastFrameTime_{};  // codex-line-comment: documents this line.
  AppConfig config_{};  // codex-line-comment: documents this line.
  ConfigStore configStore_{};  // codex-line-comment: documents this line.
  GameWindowTracker gameWindowTracker_{};  // codex-line-comment: documents this line.
  std::unique_ptr<IFrameCapture> frameCapture_{};  // codex-line-comment: documents this line.
  GearDetector gearDetector_{};  // codex-line-comment: documents this line.
  GearColorClassifier gearColorClassifier_{};  // codex-line-comment: documents this line.
  ShiftAlertController shiftAlertController_{};  // codex-line-comment: documents this line.
  OverlayWindow overlayWindow_{};  // codex-line-comment: documents this line.
  DiagnosticsService diagnostics_{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
