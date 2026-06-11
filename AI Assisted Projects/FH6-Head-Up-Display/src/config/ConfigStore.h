#pragma once  // Prevents this header from being included more than once.

#include <filesystem>  // Imports the filesystem standard library declarations used in this file.

#include "config/AppConfig.h"  // Imports project declarations from config/AppConfig.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class ConfigStore {  // Declares the ConfigStore class interface and members.
 public:  // Makes the following members part of the public API.
  explicit ConfigStore(std::filesystem::path configPath = defaultConfigPath());  // Declares the explicit ConfigStore constructor.

  [[nodiscard]] AppConfig load() const;  // Declares load and marks its return value as important.
  void save(const AppConfig& config) const;  // Declares function save for callers.
  [[nodiscard]] AppConfig restoreDefaults() const;  // Declares restoreDefaults and marks its return value as important.

  [[nodiscard]] const std::filesystem::path& configPath() const;  // Declares configPath and marks its return value as important.
  [[nodiscard]] static std::filesystem::path defaultConfigPath();  // Declares defaultConfigPath and marks its return value as important.

 private:  // Makes the following members private implementation details.
  std::filesystem::path configPath_;  // Declares configPath_ for use in this scope.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
