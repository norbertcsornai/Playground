#pragma once

#include <atomic>
#include <memory>

#include "alert/ShiftAlertController.h"
#include "capture/IFrameCapture.h"
#include "config/AppConfig.h"
#include "config/ConfigStore.h"
#include "detection/GearColorClassifier.h"
#include "detection/GearDetector.h"
#include "diagnostics/DiagnosticsService.h"
#include "overlay/OverlayWindow.h"
#include "platform/GameWindowTracker.h"
#include "shared/Time.h"

namespace fh6 {

class HudApplication {
 public:
  HudApplication();
  HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore);

  bool initialize();
  void run();
  void requestStop();
  void shutdown();

 private:
  void processFrame();
  void enterIdleMode();
  void ensureCaptureForActiveDisplay();

  std::atomic_bool running_{false};
  TimePoint lastFrameTime_{};
  AppConfig config_{};
  ConfigStore configStore_{};
  GameWindowTracker gameWindowTracker_{};
  std::unique_ptr<IFrameCapture> frameCapture_{};
  GearDetector gearDetector_{};
  GearColorClassifier gearColorClassifier_{};
  ShiftAlertController shiftAlertController_{};
  OverlayWindow overlayWindow_{};
  DiagnosticsService diagnostics_{};
};

}  // namespace fh6
