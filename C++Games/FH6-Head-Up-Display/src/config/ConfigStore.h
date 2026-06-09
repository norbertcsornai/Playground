#pragma once  // codex-line-comment: documents this line.

#include <filesystem>  // codex-line-comment: documents this line.

#include "config/AppConfig.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class ConfigStore {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  explicit ConfigStore(std::filesystem::path configPath = defaultConfigPath());  // codex-line-comment: documents this line.

  [[nodiscard]] AppConfig load() const;  // codex-line-comment: documents this line.
  void save(const AppConfig& config) const;  // codex-line-comment: documents this line.
  [[nodiscard]] AppConfig restoreDefaults() const;  // codex-line-comment: documents this line.

  [[nodiscard]] const std::filesystem::path& configPath() const;  // codex-line-comment: documents this line.
  [[nodiscard]] static std::filesystem::path defaultConfigPath();  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  std::filesystem::path configPath_;  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
