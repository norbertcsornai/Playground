#include "config/ConfigStore.h"  // Imports project declarations from config/ConfigStore.h.

#include <cstdlib>  // Imports the cstdlib standard library declarations used in this file.
#include <fstream>  // Imports the fstream standard library declarations used in this file.
#include <sstream>  // Imports the sstream standard library declarations used in this file.
#include <string>  // Imports the string standard library declarations used in this file.
#include <unordered_map>  // Imports the unordered_map standard library declarations used in this file.

namespace fh6 {  // Places the following declarations inside namespace fh6.

namespace {  // Starts a file-local helper namespace.

int readInt(const std::unordered_map<std::string, std::string>& values, const std::string& key,  // Supplies int readInt(const std::unordered_map<std::string, std::string>& values, const std::stri... to the surrounding call or initializer.
            int fallback) {  // Starts a multi-line initializer or scope for int fallback).
  const auto it = values.find(key);  // Sets const auto it to values.find(key).
  if (it == values.end()) {  // Guards the following work behind the condition it == values.end().
    return fallback;  // Returns fallback to the caller.
  }  // Ends the current code block.

  try {  // Begins work that may throw an exception.
    return std::stoi(it->second);  // Returns std::stoi(it->second) to the caller.
  } catch (...) {  // Handles the exception object declared as ....
    return fallback;  // Returns fallback to the caller.
  }  // Ends the current code block.
}  // Ends the current code block.

bool readBool(const std::unordered_map<std::string, std::string>& values, const std::string& key,  // Supplies bool readBool(const std::unordered_map<std::string, std::string>& values, const std::st... to the surrounding call or initializer.
              bool fallback) {  // Starts a multi-line initializer or scope for bool fallback).
  const auto it = values.find(key);  // Sets const auto it to values.find(key).
  if (it == values.end()) {  // Guards the following work behind the condition it == values.end().
    return fallback;  // Returns fallback to the caller.
  }  // Ends the current code block.

  return it->second == "true" || it->second == "1";  // Returns it->second == "true" || it->second == "1" to the caller.
}  // Ends the current code block.

bool isLegacyDefaultGearRegion(const Rect& region) {  // Begins function isLegacyDefaultGearRegion.
  return (region.x == 0 && region.y == 0 && region.width == 120 && region.height == 120) ||  // Continues the surrounding declaration or control-flow expression.
         (region.x == -1 && region.y == -1 && region.width == 260 && region.height == 220);  // Sets (region.x to = -1 && region.y == -1 && region.width == 260 && region.height == 220).
}  // Ends the current code block.

bool isLegacyDefaultArrowSize(const Size& size) {  // Begins function isLegacyDefaultArrowSize.
  return (size.width == 50 && size.height == 100) ||  // Continues the surrounding declaration or control-flow expression.
         (size.width == 300 && size.height == 600) ||  // Continues the surrounding declaration or control-flow expression.
         (size.width == 150 && size.height == 300);  // Sets (size.width to = 150 && size.height == 300).
}  // Ends the current code block.

bool isLegacyDefaultArrowDuration(std::chrono::milliseconds duration) {  // Begins function isLegacyDefaultArrowDuration.
  return duration == std::chrono::milliseconds(250);  // Returns duration == std::chrono::milliseconds(250) to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

ConfigStore::ConfigStore(std::filesystem::path configPath) : configPath_(std::move(configPath)) {}  // Finishes this initializer entry for the surrounding aggregate.

AppConfig ConfigStore::load() const {  // Implements ConfigStore::load.
  AppConfig config{};  // Declares config with value initialization.
  std::ifstream input(configPath_);  // Declares function input for callers.
  if (!input) {  // Guards the following work behind the condition !input.
    return config;  // Returns config to the caller.
  }  // Ends the current code block.

  std::unordered_map<std::string, std::string> values;  // Declares values for use in this scope.
  std::string line;  // Declares line for use in this scope.
  while (std::getline(input, line)) {  // Repeats while std::getline(input, line) remains true.
    if (line.empty() || line[0] == '#') {  // Guards the following work behind the condition line.empty() || line[0] == '#'.
      continue;  // Skips the rest of this loop iteration.
    }  // Ends the current code block.

    const auto equals = line.find('=');  // Sets const auto equals to line.find('=').
    if (equals == std::string::npos) {  // Guards the following work behind the condition equals == std::string::npos.
      continue;  // Skips the rest of this loop iteration.
    }  // Ends the current code block.

    values[line.substr(0, equals)] = line.substr(equals + 1);  // Sets values[line.substr(0, equals)] to line.substr(equals + 1).
  }  // Ends the current code block.

  config.gearRegion.x = readInt(values, "gearRegion.x", config.gearRegion.x);  // Sets config.gearRegion.x to readInt(values, "gearRegion.x", config.gearRegion.x).
  config.gearRegion.y = readInt(values, "gearRegion.y", config.gearRegion.y);  // Sets config.gearRegion.y to readInt(values, "gearRegion.y", config.gearRegion.y).
  config.gearRegion.width = readInt(values, "gearRegion.width", config.gearRegion.width);  // Sets config.gearRegion.width to readInt(values, "gearRegion.width", config.gearRegion.width).
  config.gearRegion.height = readInt(values, "gearRegion.height", config.gearRegion.height);  // Sets config.gearRegion.height to readInt(values, "gearRegion.height", config.gearRegion.height).
  config.arrowDuration = std::chrono::milliseconds(readInt(values, "arrowDurationMs", 250));  // Sets config.arrowDuration to std::chrono::milliseconds(readInt(values, "arrowDurationMs", 250)).
  config.arrowSize.width = readInt(values, "arrowSize.width", config.arrowSize.width);  // Sets config.arrowSize.width to readInt(values, "arrowSize.width", config.arrowSize.width).
  config.arrowSize.height = readInt(values, "arrowSize.height", config.arrowSize.height);  // Sets config.arrowSize.height to readInt(values, "arrowSize.height", config.arrowSize.height).
  config.captureRateLimitFps = readInt(values, "captureRateLimitFps", config.captureRateLimitFps);  // Sets config.captureRateLimitFps to readInt(values, "captureRateLimitFps", config.captureRateLimitFps).
  config.diagnosticsEnabled = readBool(values, "diagnosticsEnabled", config.diagnosticsEnabled);  // Sets config.diagnosticsEnabled to readBool(values, "diagnosticsEnabled", config.diagnosticsEnabled).

  if (isLegacyDefaultGearRegion(config.gearRegion)) {  // Guards the following work behind the condition isLegacyDefaultGearRegion(config.gearRegion).
    config.gearRegion = Rect{-1, -1, 420, 420};  // Sets config.gearRegion to Rect{-1, -1, 420, 420}.
  }  // Ends the current code block.

  if (isLegacyDefaultArrowSize(config.arrowSize)) {  // Guards the following work behind the condition isLegacyDefaultArrowSize(config.arrowSize).
    config.arrowSize = Size{75, 150};  // Sets config.arrowSize to Size{75, 150}.
  }  // Ends the current code block.

  if (isLegacyDefaultArrowDuration(config.arrowDuration)) {  // Guards the following work behind the condition isLegacyDefaultArrowDuration(config.arrowDuration).
    config.arrowDuration = std::chrono::milliseconds(1500);  // Sets config.arrowDuration to std::chrono::milliseconds(1500).
  }  // Ends the current code block.

  if (const auto it = values.find("targetDisplayId"); it != values.end()) {  // Guards the following work behind the condition const auto it = values.find("targetDisplayId"); it != values.end().
    config.targetDisplayId = it->second;  // Sets config.targetDisplayId to it->second.
  }  // Ends the current code block.

  return config;  // Returns config to the caller.
}  // Ends the current code block.

void ConfigStore::save(const AppConfig& config) const {  // Implements ConfigStore::save.
  if (const auto parent = configPath_.parent_path(); !parent.empty()) {  // Guards the following work behind the condition const auto parent = configPath_.parent_path(); !parent.empty().
    std::filesystem::create_directories(parent);  // Executes std::filesystem::create_directories(parent).
  }  // Ends the current code block.

  std::ofstream output(configPath_);  // Declares function output for callers.
  output << "gearRegion.x=" << config.gearRegion.x << '\n';  // Sets output << "gearRegion.x to " << config.gearRegion.x << '\n'.
  output << "gearRegion.y=" << config.gearRegion.y << '\n';  // Sets output << "gearRegion.y to " << config.gearRegion.y << '\n'.
  output << "gearRegion.width=" << config.gearRegion.width << '\n';  // Sets output << "gearRegion.width to " << config.gearRegion.width << '\n'.
  output << "gearRegion.height=" << config.gearRegion.height << '\n';  // Sets output << "gearRegion.height to " << config.gearRegion.height << '\n'.
  output << "arrowDurationMs=" << config.arrowDuration.count() << '\n';  // Sets output << "arrowDurationMs to " << config.arrowDuration.count() << '\n'.
  output << "arrowSize.width=" << config.arrowSize.width << '\n';  // Sets output << "arrowSize.width to " << config.arrowSize.width << '\n'.
  output << "arrowSize.height=" << config.arrowSize.height << '\n';  // Sets output << "arrowSize.height to " << config.arrowSize.height << '\n'.
  output << "targetDisplayId=" << config.targetDisplayId << '\n';  // Sets output << "targetDisplayId to " << config.targetDisplayId << '\n'.
  output << "captureRateLimitFps=" << config.captureRateLimitFps << '\n';  // Sets output << "captureRateLimitFps to " << config.captureRateLimitFps << '\n'.
  output << "diagnosticsEnabled=" << (config.diagnosticsEnabled ? "true" : "false") << '\n';  // Sets output << "diagnosticsEnabled to " << (config.diagnosticsEnabled ? "true" : "false") << '\n'.
}  // Ends the current code block.

AppConfig ConfigStore::restoreDefaults() const {  // Implements ConfigStore::restoreDefaults.
  AppConfig config{};  // Declares config with value initialization.
  save(config);  // Invokes save with the supplied arguments.
  return config;  // Returns config to the caller.
}  // Ends the current code block.

const std::filesystem::path& ConfigStore::configPath() const {  // Implements ConfigStore::configPath.
  return configPath_;  // Returns configPath_ to the caller.
}  // Ends the current code block.

std::filesystem::path ConfigStore::defaultConfigPath() {  // Implements ConfigStore::defaultConfigPath.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  if (const char* appData = std::getenv("APPDATA")) {  // Guards the following work behind the condition const char* appData = std::getenv("APPDATA").
    return std::filesystem::path(appData) / "FH6HeadUpDisplay" / "config.ini";  // Returns std::filesystem::path(appData) / "FH6HeadUpDisplay" / "config.ini" to the caller.
  }  // Ends the current code block.
#endif  // Ends the compile-time selection block.
  return std::filesystem::path("fh6-hud-config.ini");  // Returns std::filesystem::path("fh6-hud-config.ini") to the caller.
}  // Ends the current code block.

}  // Ends the current code block.
