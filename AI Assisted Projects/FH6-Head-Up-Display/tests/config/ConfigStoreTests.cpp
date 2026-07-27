#include "TestFramework.h"  // Imports project declarations from TestFramework.h.

#include <filesystem>  // Imports the filesystem standard library declarations used in this file.

#include "config/ConfigStore.h"  // Imports project declarations from config/ConfigStore.h.

using namespace fh6;  // Declares fh6 for use in this scope.

namespace {  // Starts a file-local helper namespace.

std::filesystem::path tempConfigPath(const char* name) {  // Begins function tempConfigPath.
  return std::filesystem::temp_directory_path() / name;  // Returns std::filesystem::temp_directory_path() / name to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

FH6_TEST(config_store_loads_defaults_when_missing) {  // Starts a multi-line initializer or scope for FH6_TEST(config_store_loads_defaults_when_missing).
  const auto path = tempConfigPath("fh6-hud-missing-test.ini");  // Sets const auto path to tempConfigPath("fh6-hud-missing-test.ini").
  std::filesystem::remove(path);  // Executes std::filesystem::remove(path).

  ConfigStore store(path);  // Declares function store for callers.
  const auto config = store.load();  // Sets const auto config to store.load().

  FH6_REQUIRE(config.arrowSize.width == 75);  // Sets FH6_REQUIRE(config.arrowSize.width to = 75).
  FH6_REQUIRE(config.arrowSize.height == 150);  // Sets FH6_REQUIRE(config.arrowSize.height to = 150).
  FH6_REQUIRE(config.gearRegion.width == 420);  // Sets FH6_REQUIRE(config.gearRegion.width to = 420).
  FH6_REQUIRE(config.gearRegion.height == 420);  // Sets FH6_REQUIRE(config.gearRegion.height to = 420).
  FH6_REQUIRE(config.arrowDuration.count() == 1500);  // Sets FH6_REQUIRE(config.arrowDuration.count() to = 1500).
  FH6_REQUIRE(config.captureRateLimitFps == 120);  // Sets FH6_REQUIRE(config.captureRateLimitFps to = 120).
}  // Ends the current code block.

FH6_TEST(config_store_saves_and_loads_values) {  // Starts a multi-line initializer or scope for FH6_TEST(config_store_saves_and_loads_values).
  const auto path = tempConfigPath("fh6-hud-save-test.ini");  // Sets const auto path to tempConfigPath("fh6-hud-save-test.ini").
  std::filesystem::remove(path);  // Executes std::filesystem::remove(path).

  ConfigStore store(path);  // Declares function store for callers.
  AppConfig config;  // Declares config for use in this scope.
  config.gearRegion = Rect{4, 5, 60, 70};  // Sets config.gearRegion to Rect{4, 5, 60, 70}.
  config.arrowDuration = std::chrono::milliseconds(400);  // Sets config.arrowDuration to std::chrono::milliseconds(400).
  config.diagnosticsEnabled = true;  // Sets config.diagnosticsEnabled to true.
  store.save(config);  // Calls save on store.

  const auto loaded = store.load();  // Sets const auto loaded to store.load().

  FH6_REQUIRE(loaded.gearRegion.x == 4);  // Sets FH6_REQUIRE(loaded.gearRegion.x to = 4).
  FH6_REQUIRE(loaded.gearRegion.y == 5);  // Sets FH6_REQUIRE(loaded.gearRegion.y to = 5).
  FH6_REQUIRE(loaded.gearRegion.width == 60);  // Sets FH6_REQUIRE(loaded.gearRegion.width to = 60).
  FH6_REQUIRE(loaded.gearRegion.height == 70);  // Sets FH6_REQUIRE(loaded.gearRegion.height to = 70).
  FH6_REQUIRE(loaded.arrowDuration.count() == 400);  // Sets FH6_REQUIRE(loaded.arrowDuration.count() to = 400).
  FH6_REQUIRE(loaded.diagnosticsEnabled);  // Invokes FH6_REQUIRE with the supplied arguments.

  std::filesystem::remove(path);  // Executes std::filesystem::remove(path).
}  // Ends the current code block.
