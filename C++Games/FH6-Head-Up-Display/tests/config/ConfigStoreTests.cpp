#include "TestFramework.h"

#include <filesystem>

#include "config/ConfigStore.h"

using namespace fh6;

namespace {

std::filesystem::path tempConfigPath(const char* name) {
  return std::filesystem::temp_directory_path() / name;
}

}  // namespace

FH6_TEST(config_store_loads_defaults_when_missing) {
  const auto path = tempConfigPath("fh6-hud-missing-test.ini");
  std::filesystem::remove(path);

  ConfigStore store(path);
  const auto config = store.load();

  FH6_REQUIRE(config.arrowSize.width == 150);
  FH6_REQUIRE(config.arrowSize.height == 300);
  FH6_REQUIRE(config.gearRegion.width == 420);
  FH6_REQUIRE(config.gearRegion.height == 420);
  FH6_REQUIRE(config.arrowDuration.count() == 1500);
  FH6_REQUIRE(config.captureRateLimitFps == 30);
}

FH6_TEST(config_store_saves_and_loads_values) {
  const auto path = tempConfigPath("fh6-hud-save-test.ini");
  std::filesystem::remove(path);

  ConfigStore store(path);
  AppConfig config;
  config.gearRegion = Rect{4, 5, 60, 70};
  config.arrowDuration = std::chrono::milliseconds(400);
  config.diagnosticsEnabled = true;
  store.save(config);

  const auto loaded = store.load();

  FH6_REQUIRE(loaded.gearRegion.x == 4);
  FH6_REQUIRE(loaded.gearRegion.y == 5);
  FH6_REQUIRE(loaded.gearRegion.width == 60);
  FH6_REQUIRE(loaded.gearRegion.height == 70);
  FH6_REQUIRE(loaded.arrowDuration.count() == 400);
  FH6_REQUIRE(loaded.diagnosticsEnabled);

  std::filesystem::remove(path);
}
