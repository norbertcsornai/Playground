#pragma once  // Prevents this header from being included more than once.

#include <atomic>  // Imports the atomic standard library declarations used in this file.
#include <chrono>  // Imports the chrono standard library declarations used in this file.
#include <memory>  // Imports the memory standard library declarations used in this file.

#include "alert/ShiftAlertController.h"  // Imports project declarations from alert/ShiftAlertController.h.
#include "capture/IFrameCapture.h"  // Imports project declarations from capture/IFrameCapture.h.
#include "config/AppConfig.h"  // Imports project declarations from config/AppConfig.h.
#include "config/ConfigStore.h"  // Imports project declarations from config/ConfigStore.h.
#include "detection/GearColorClassifier.h"  // Imports project declarations from detection/GearColorClassifier.h.
#include "detection/GearDetector.h"  // Imports project declarations from detection/GearDetector.h.
#include "diagnostics/DiagnosticsService.h"  // Imports project declarations from diagnostics/DiagnosticsService.h.
#include "overlay/OverlayWindow.h"  // Imports project declarations from overlay/OverlayWindow.h.
#include "platform/GameWindowTracker.h"  // Imports project declarations from platform/GameWindowTracker.h.
#include "shared/Time.h"  // Imports project declarations from shared/Time.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class HudApplication {  // Declares the HudApplication class interface and members.
 public:  // Makes the following members part of the public API.
  HudApplication();  // Invokes HudApplication with the supplied arguments.
  HudApplication(std::unique_ptr<IFrameCapture> frameCapture, ConfigStore configStore);  // Invokes HudApplication with the supplied arguments.

  bool initialize();  // Declares function initialize for callers.
  void run();  // Declares function run for callers.
  void requestStop();  // Declares function requestStop for callers.
  void shutdown();  // Declares function shutdown for callers.

 private:  // Makes the following members private implementation details.
  void processFrame();  // Declares function processFrame for callers.
  void enterIdleMode();  // Declares function enterIdleMode for callers.
  void ensureCaptureForActiveDisplay();  // Declares function ensureCaptureForActiveDisplay for callers.

  // How often the expensive top-level window scan may run. Short enough that the game is picked up
  // promptly after launch, long enough that the scan is not a per-frame cost.
  static constexpr std::chrono::milliseconds kWindowScanInterval{250};  // Defines the minimum gap between window scans.

  std::atomic_bool running_{false};  // Declares running_ and initializes it with false.
  TimePoint lastFrameTime_{};  // Declares lastFrameTime_ with value initialization.
  TimePoint lastWindowScan_{};  // Declares when the top-level window scan last ran.
  AppConfig config_{};  // Declares config_ with value initialization.
  ConfigStore configStore_{};  // Declares configStore_ with value initialization.
  GameWindowTracker gameWindowTracker_{};  // Declares gameWindowTracker_ with value initialization.
  std::unique_ptr<IFrameCapture> frameCapture_{};  // Declares frameCapture_ with value initialization.
  GearDetector gearDetector_{};  // Declares gearDetector_ with value initialization.
  GearColorClassifier gearColorClassifier_{};  // Declares gearColorClassifier_ with value initialization.
  ShiftAlertController shiftAlertController_{};  // Declares shiftAlertController_ with value initialization.
  OverlayWindow overlayWindow_{};  // Declares overlayWindow_ with value initialization.
  DiagnosticsService diagnostics_{};  // Declares diagnostics_ with value initialization.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
