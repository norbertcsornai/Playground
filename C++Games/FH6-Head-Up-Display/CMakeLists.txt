cmake_minimum_required(VERSION 3.18)

project(FH6HeadUpDisplay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(fh6_hud_core
  src/alert/ShiftAlertController.cpp
  src/app/HudApplication.cpp
  src/capture/DesktopFrameCapture.cpp
  src/capture/Frame.cpp
  src/config/ConfigStore.cpp
  src/detection/CalibrationService.cpp
  src/detection/GearColorClassifier.cpp
  src/detection/GearDetector.cpp
  src/diagnostics/DiagnosticsService.cpp
  src/overlay/ArrowRenderer.cpp
  src/overlay/OverlayWindow.cpp
  src/platform/GameWindowTracker.cpp
)

target_include_directories(fh6_hud_core PUBLIC src)

if(WIN32)
  target_link_libraries(fh6_hud_core PRIVATE user32 gdi32 dwmapi d3d11 dxgi)
  target_compile_definitions(fh6_hud_core PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

add_executable(fh6-hud src/main.cpp)
target_link_libraries(fh6-hud PRIVATE fh6_hud_core)

enable_testing()

add_executable(fh6-hud-tests
  tests/TestMain.cpp
  tests/alert/ShiftAlertControllerTests.cpp
  tests/config/ConfigStoreTests.cpp
  tests/detection/GearColorClassifierTests.cpp
  tests/detection/GearDetectorTests.cpp
)

target_link_libraries(fh6-hud-tests PRIVATE fh6_hud_core)
target_include_directories(fh6-hud-tests PRIVATE tests)
add_test(NAME fh6-hud-tests COMMAND fh6-hud-tests)
