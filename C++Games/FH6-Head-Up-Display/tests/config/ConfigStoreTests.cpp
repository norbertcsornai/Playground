#include "TestFramework.h"  // codex-line-comment: documents this line.

#include <filesystem>  // codex-line-comment: documents this line.

#include "config/ConfigStore.h"  // codex-line-comment: documents this line.

using namespace fh6;  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

std::filesystem::path tempConfigPath(const char* name) {  // codex-line-comment: documents this line.
  return std::filesystem::temp_directory_path() / name;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

FH6_TEST(config_store_loads_defaults_when_missing) {  // codex-line-comment: documents this line.
  const auto path = tempConfigPath("fh6-hud-missing-test.ini");  // codex-line-comment: documents this line.
  std::filesystem::remove(path);  // codex-line-comment: documents this line.

  ConfigStore store(path);  // codex-line-comment: documents this line.
  const auto config = store.load();  // codex-line-comment: documents this line.

  FH6_REQUIRE(config.arrowSize.width == 75);  // codex-line-comment: documents this line.
  FH6_REQUIRE(config.arrowSize.height == 150);  // codex-line-comment: documents this line.
  FH6_REQUIRE(config.gearRegion.width == 420);  // codex-line-comment: documents this line.
  FH6_REQUIRE(config.gearRegion.height == 420);  // codex-line-comment: documents this line.
  FH6_REQUIRE(config.arrowDuration.count() == 1500);  // codex-line-comment: documents this line.
  FH6_REQUIRE(config.captureRateLimitFps == 30);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(config_store_saves_and_loads_values) {  // codex-line-comment: documents this line.
  const auto path = tempConfigPath("fh6-hud-save-test.ini");  // codex-line-comment: documents this line.
  std::filesystem::remove(path);  // codex-line-comment: documents this line.

  ConfigStore store(path);  // codex-line-comment: documents this line.
  AppConfig config;  // codex-line-comment: documents this line.
  config.gearRegion = Rect{4, 5, 60, 70};  // codex-line-comment: documents this line.
  config.arrowDuration = std::chrono::milliseconds(400);  // codex-line-comment: documents this line.
  config.diagnosticsEnabled = true;  // codex-line-comment: documents this line.
  store.save(config);  // codex-line-comment: documents this line.

  const auto loaded = store.load();  // codex-line-comment: documents this line.

  FH6_REQUIRE(loaded.gearRegion.x == 4);  // codex-line-comment: documents this line.
  FH6_REQUIRE(loaded.gearRegion.y == 5);  // codex-line-comment: documents this line.
  FH6_REQUIRE(loaded.gearRegion.width == 60);  // codex-line-comment: documents this line.
  FH6_REQUIRE(loaded.gearRegion.height == 70);  // codex-line-comment: documents this line.
  FH6_REQUIRE(loaded.arrowDuration.count() == 400);  // codex-line-comment: documents this line.
  FH6_REQUIRE(loaded.diagnosticsEnabled);  // codex-line-comment: documents this line.

  std::filesystem::remove(path);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.
