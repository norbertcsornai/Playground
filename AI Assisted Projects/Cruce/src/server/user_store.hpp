#pragma once

#include "core/types.hpp"

#include <filesystem>
#include <map>
#include <string>

namespace cruce::server {

struct UserRecord {
  std::string username;
  std::string password_hash;
};

class UserStore {
 public:
  explicit UserStore(std::filesystem::path database_path);

  ValidationResult initialize();
  ValidationResult register_user(const std::string& username, const std::string& password);
  ValidationResult authenticate(const std::string& username, const std::string& password) const;
  bool exists(const std::string& username) const;
  const std::map<std::string, UserRecord>& users() const;

 private:
  static bool valid_credential_text(const std::string& text);
  static std::string password_hash(const std::string& username, const std::string& password);

  ValidationResult load();
  ValidationResult save() const;
  void add_initial_user_if_missing(const std::string& username, const std::string& password);

  std::filesystem::path database_path_;
  std::map<std::string, UserRecord> users_;
};

}  // namespace cruce::server
