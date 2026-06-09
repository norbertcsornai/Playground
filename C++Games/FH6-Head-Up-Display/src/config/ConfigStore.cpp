#include "config/ConfigStore.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace fh6 {

namespace {

int readInt(const std::unordered_map<std::string, std::string>& values, const std::string& key,
            int fallback) {
  const auto it = values.find(key);
  if (it == values.end()) {
    return fallback;
  }

  try {
    return std::stoi(it->second);
  } catch (...) {
    return fallback;
  }
}

bool readBool(const std::unordered_map<std::string, std::string>& values, const std::string& key,
              bool fallback) {
  const auto it = values.find(key);
  if (it == values.end()) {
    return fallback;
  }

  return it->second == "true" || it->second == "1";
}

bool isLegacyDefaultGearRegion(const Rect& region) {
  return (region.x == 0 && region.y == 0 && region.width == 120 && region.height == 120) ||
         (region.x == -1 && region.y == -1 && region.width == 260 && region.height == 220);
}

bool isLegacyDefaultArrowSize(const Size& size) {
  return (size.width == 50 && size.height == 100) ||
         (size.width == 300 && size.height == 600);
}

bool isLegacyDefaultArrowDuration(std::chrono::milliseconds duration) {
  return duration == std::chrono::milliseconds(250);
}

}  // namespace

ConfigStore::ConfigStore(std::filesystem::path configPath) : configPath_(std::move(configPath)) {}

AppConfig ConfigStore::load() const {
  AppConfig config{};
  std::ifstream input(configPath_);
  if (!input) {
    return config;
  }

  std::unordered_map<std::string, std::string> values;
  std::string line;
  while (std::getline(input, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    const auto equals = line.find('=');
    if (equals == std::string::npos) {
      continue;
    }

    values[line.substr(0, equals)] = line.substr(equals + 1);
  }

  config.gearRegion.x = readInt(values, "gearRegion.x", config.gearRegion.x);
  config.gearRegion.y = readInt(values, "gearRegion.y", config.gearRegion.y);
  config.gearRegion.width = readInt(values, "gearRegion.width", config.gearRegion.width);
  config.gearRegion.height = readInt(values, "gearRegion.height", config.gearRegion.height);
  config.arrowDuration = std::chrono::milliseconds(readInt(values, "arrowDurationMs", 250));
  config.arrowSize.width = readInt(values, "arrowSize.width", config.arrowSize.width);
  config.arrowSize.height = readInt(values, "arrowSize.height", config.arrowSize.height);
  config.captureRateLimitFps = readInt(values, "captureRateLimitFps", config.captureRateLimitFps);
  config.diagnosticsEnabled = readBool(values, "diagnosticsEnabled", config.diagnosticsEnabled);

  if (isLegacyDefaultGearRegion(config.gearRegion)) {
    config.gearRegion = Rect{-1, -1, 420, 420};
  }

  if (isLegacyDefaultArrowSize(config.arrowSize)) {
    config.arrowSize = Size{150, 300};
  }

  if (isLegacyDefaultArrowDuration(config.arrowDuration)) {
    config.arrowDuration = std::chrono::milliseconds(1500);
  }

  if (const auto it = values.find("targetDisplayId"); it != values.end()) {
    config.targetDisplayId = it->second;
  }

  return config;
}

void ConfigStore::save(const AppConfig& config) const {
  if (const auto parent = configPath_.parent_path(); !parent.empty()) {
    std::filesystem::create_directories(parent);
  }

  std::ofstream output(configPath_);
  output << "gearRegion.x=" << config.gearRegion.x << '\n';
  output << "gearRegion.y=" << config.gearRegion.y << '\n';
  output << "gearRegion.width=" << config.gearRegion.width << '\n';
  output << "gearRegion.height=" << config.gearRegion.height << '\n';
  output << "arrowDurationMs=" << config.arrowDuration.count() << '\n';
  output << "arrowSize.width=" << config.arrowSize.width << '\n';
  output << "arrowSize.height=" << config.arrowSize.height << '\n';
  output << "targetDisplayId=" << config.targetDisplayId << '\n';
  output << "captureRateLimitFps=" << config.captureRateLimitFps << '\n';
  output << "diagnosticsEnabled=" << (config.diagnosticsEnabled ? "true" : "false") << '\n';
}

AppConfig ConfigStore::restoreDefaults() const {
  AppConfig config{};
  save(config);
  return config;
}

const std::filesystem::path& ConfigStore::configPath() const {
  return configPath_;
}

std::filesystem::path ConfigStore::defaultConfigPath() {
#ifdef _WIN32
  if (const char* appData = std::getenv("APPDATA")) {
    return std::filesystem::path(appData) / "FH6HeadUpDisplay" / "config.ini";
  }
#endif
  return std::filesystem::path("fh6-hud-config.ini");
}

}  // namespace fh6
