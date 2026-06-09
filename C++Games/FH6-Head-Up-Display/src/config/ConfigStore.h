#pragma once

#include <filesystem>

#include "config/AppConfig.h"

namespace fh6 {

class ConfigStore {
 public:
  explicit ConfigStore(std::filesystem::path configPath = defaultConfigPath());

  [[nodiscard]] AppConfig load() const;
  void save(const AppConfig& config) const;
  [[nodiscard]] AppConfig restoreDefaults() const;

  [[nodiscard]] const std::filesystem::path& configPath() const;
  [[nodiscard]] static std::filesystem::path defaultConfigPath();

 private:
  std::filesystem::path configPath_;
};

}  // namespace fh6
