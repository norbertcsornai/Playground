#include "config/ConfigStore.h"  // codex-line-comment: documents this line.

#include <cstdlib>  // codex-line-comment: documents this line.
#include <fstream>  // codex-line-comment: documents this line.
#include <sstream>  // codex-line-comment: documents this line.
#include <string>  // codex-line-comment: documents this line.
#include <unordered_map>  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

int readInt(const std::unordered_map<std::string, std::string>& values, const std::string& key,  // codex-line-comment: documents this line.
            int fallback) {  // codex-line-comment: documents this line.
  const auto it = values.find(key);  // codex-line-comment: documents this line.
  if (it == values.end()) {  // codex-line-comment: documents this line.
    return fallback;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  try {  // codex-line-comment: documents this line.
    return std::stoi(it->second);  // codex-line-comment: documents this line.
  } catch (...) {  // codex-line-comment: documents this line.
    return fallback;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool readBool(const std::unordered_map<std::string, std::string>& values, const std::string& key,  // codex-line-comment: documents this line.
              bool fallback) {  // codex-line-comment: documents this line.
  const auto it = values.find(key);  // codex-line-comment: documents this line.
  if (it == values.end()) {  // codex-line-comment: documents this line.
    return fallback;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return it->second == "true" || it->second == "1";  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool isLegacyDefaultGearRegion(const Rect& region) {  // codex-line-comment: documents this line.
  return (region.x == 0 && region.y == 0 && region.width == 120 && region.height == 120) ||  // codex-line-comment: documents this line.
         (region.x == -1 && region.y == -1 && region.width == 260 && region.height == 220);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool isLegacyDefaultArrowSize(const Size& size) {  // codex-line-comment: documents this line.
  return (size.width == 50 && size.height == 100) ||  // codex-line-comment: documents this line.
         (size.width == 300 && size.height == 600) ||  // codex-line-comment: documents this line.
         (size.width == 150 && size.height == 300);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool isLegacyDefaultArrowDuration(std::chrono::milliseconds duration) {  // codex-line-comment: documents this line.
  return duration == std::chrono::milliseconds(250);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

ConfigStore::ConfigStore(std::filesystem::path configPath) : configPath_(std::move(configPath)) {}  // codex-line-comment: documents this line.

AppConfig ConfigStore::load() const {  // codex-line-comment: documents this line.
  AppConfig config{};  // codex-line-comment: documents this line.
  std::ifstream input(configPath_);  // codex-line-comment: documents this line.
  if (!input) {  // codex-line-comment: documents this line.
    return config;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::unordered_map<std::string, std::string> values;  // codex-line-comment: documents this line.
  std::string line;  // codex-line-comment: documents this line.
  while (std::getline(input, line)) {  // codex-line-comment: documents this line.
    if (line.empty() || line[0] == '#') {  // codex-line-comment: documents this line.
      continue;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.

    const auto equals = line.find('=');  // codex-line-comment: documents this line.
    if (equals == std::string::npos) {  // codex-line-comment: documents this line.
      continue;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.

    values[line.substr(0, equals)] = line.substr(equals + 1);  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  config.gearRegion.x = readInt(values, "gearRegion.x", config.gearRegion.x);  // codex-line-comment: documents this line.
  config.gearRegion.y = readInt(values, "gearRegion.y", config.gearRegion.y);  // codex-line-comment: documents this line.
  config.gearRegion.width = readInt(values, "gearRegion.width", config.gearRegion.width);  // codex-line-comment: documents this line.
  config.gearRegion.height = readInt(values, "gearRegion.height", config.gearRegion.height);  // codex-line-comment: documents this line.
  config.arrowDuration = std::chrono::milliseconds(readInt(values, "arrowDurationMs", 250));  // codex-line-comment: documents this line.
  config.arrowSize.width = readInt(values, "arrowSize.width", config.arrowSize.width);  // codex-line-comment: documents this line.
  config.arrowSize.height = readInt(values, "arrowSize.height", config.arrowSize.height);  // codex-line-comment: documents this line.
  config.captureRateLimitFps = readInt(values, "captureRateLimitFps", config.captureRateLimitFps);  // codex-line-comment: documents this line.
  config.diagnosticsEnabled = readBool(values, "diagnosticsEnabled", config.diagnosticsEnabled);  // codex-line-comment: documents this line.

  if (isLegacyDefaultGearRegion(config.gearRegion)) {  // codex-line-comment: documents this line.
    config.gearRegion = Rect{-1, -1, 420, 420};  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (isLegacyDefaultArrowSize(config.arrowSize)) {  // codex-line-comment: documents this line.
    config.arrowSize = Size{75, 150};  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (isLegacyDefaultArrowDuration(config.arrowDuration)) {  // codex-line-comment: documents this line.
    config.arrowDuration = std::chrono::milliseconds(1500);  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (const auto it = values.find("targetDisplayId"); it != values.end()) {  // codex-line-comment: documents this line.
    config.targetDisplayId = it->second;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return config;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void ConfigStore::save(const AppConfig& config) const {  // codex-line-comment: documents this line.
  if (const auto parent = configPath_.parent_path(); !parent.empty()) {  // codex-line-comment: documents this line.
    std::filesystem::create_directories(parent);  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::ofstream output(configPath_);  // codex-line-comment: documents this line.
  output << "gearRegion.x=" << config.gearRegion.x << '\n';  // codex-line-comment: documents this line.
  output << "gearRegion.y=" << config.gearRegion.y << '\n';  // codex-line-comment: documents this line.
  output << "gearRegion.width=" << config.gearRegion.width << '\n';  // codex-line-comment: documents this line.
  output << "gearRegion.height=" << config.gearRegion.height << '\n';  // codex-line-comment: documents this line.
  output << "arrowDurationMs=" << config.arrowDuration.count() << '\n';  // codex-line-comment: documents this line.
  output << "arrowSize.width=" << config.arrowSize.width << '\n';  // codex-line-comment: documents this line.
  output << "arrowSize.height=" << config.arrowSize.height << '\n';  // codex-line-comment: documents this line.
  output << "targetDisplayId=" << config.targetDisplayId << '\n';  // codex-line-comment: documents this line.
  output << "captureRateLimitFps=" << config.captureRateLimitFps << '\n';  // codex-line-comment: documents this line.
  output << "diagnosticsEnabled=" << (config.diagnosticsEnabled ? "true" : "false") << '\n';  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

AppConfig ConfigStore::restoreDefaults() const {  // codex-line-comment: documents this line.
  AppConfig config{};  // codex-line-comment: documents this line.
  save(config);  // codex-line-comment: documents this line.
  return config;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

const std::filesystem::path& ConfigStore::configPath() const {  // codex-line-comment: documents this line.
  return configPath_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::filesystem::path ConfigStore::defaultConfigPath() {  // codex-line-comment: documents this line.
#ifdef _WIN32  // codex-line-comment: documents this line.
  if (const char* appData = std::getenv("APPDATA")) {  // codex-line-comment: documents this line.
    return std::filesystem::path(appData) / "FH6HeadUpDisplay" / "config.ini";  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
  return std::filesystem::path("fh6-hud-config.ini");  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
