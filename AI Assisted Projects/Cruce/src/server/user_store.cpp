#include "server/user_store.hpp"

#include <cstdint>
#include <fstream>
#include <sstream>

namespace cruce::server {

namespace {

constexpr std::uint64_t fnv_offset = 1469598103934665603ull;
constexpr std::uint64_t fnv_prime = 1099511628211ull;

}  // namespace

UserStore::UserStore(std::filesystem::path database_path)
    : database_path_(std::move(database_path)) {}

ValidationResult UserStore::initialize() {
  const auto loaded = load();
  if (!loaded) {
    return loaded;
  }

  add_initial_user_if_missing("admin1", "admin1");
  add_initial_user_if_missing("admin2", "admin2");
  return save();
}

ValidationResult UserStore::register_user(
    const std::string& username,
    const std::string& password) {
  if (!valid_credential_text(username) || !valid_credential_text(password)) {
    return ValidationResult::failure(
        "Username and password must be 1-32 letters, digits, dots, dashes, or underscores.");
  }
  if (exists(username)) {
    return ValidationResult::failure("Username already exists.");
  }

  users_[username] = UserRecord{username, password_hash(username, password)};
  return save();
}

ValidationResult UserStore::authenticate(
    const std::string& username,
    const std::string& password) const {
  const auto found = users_.find(username);
  if (found == users_.end()) {
    return ValidationResult::failure("Unknown username.");
  }

  if (found->second.password_hash != password_hash(username, password)) {
    return ValidationResult::failure("Invalid password.");
  }

  return ValidationResult::success("Login successful.");
}

bool UserStore::exists(const std::string& username) const {
  return users_.contains(username);
}

const std::map<std::string, UserRecord>& UserStore::users() const {
  return users_;
}

bool UserStore::valid_credential_text(const std::string& text) {
  if (text.empty() || text.size() > 32) {
    return false;
  }

  for (const char ch : text) {
    const bool lower = ch >= 'a' && ch <= 'z';
    const bool upper = ch >= 'A' && ch <= 'Z';
    const bool digit = ch >= '0' && ch <= '9';
    const bool safe_symbol = ch == '.' || ch == '-' || ch == '_';
    if (!lower && !upper && !digit && !safe_symbol) {
      return false;
    }
  }
  return true;
}

std::string UserStore::password_hash(
    const std::string& username,
    const std::string& password) {
  const std::string input = username + ":cruce-local-auth:" + password;
  std::uint64_t hash = fnv_offset;
  for (const unsigned char ch : input) {
    hash ^= ch;
    hash *= fnv_prime;
  }

  std::ostringstream out;
  out << std::hex << hash;
  return out.str();
}

ValidationResult UserStore::load() {
  users_.clear();
  if (!std::filesystem::exists(database_path_)) {
    return ValidationResult::success("User database will be created.");
  }

  std::ifstream input(database_path_);
  if (!input) {
    return ValidationResult::failure("Unable to open user database.");
  }

  std::string line;
  while (std::getline(input, line)) {
    if (line.empty() || line.starts_with("#")) {
      continue;
    }
    const auto separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }
    const auto username = line.substr(0, separator);
    const auto hash = line.substr(separator + 1);
    if (!username.empty() && !hash.empty()) {
      users_[username] = UserRecord{username, hash};
    }
  }

  return ValidationResult::success("User database loaded.");
}

ValidationResult UserStore::save() const {
  const auto parent = database_path_.parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(parent);
  }
  std::ofstream output(database_path_, std::ios::trunc);
  if (!output) {
    return ValidationResult::failure("Unable to save user database.");
  }

  output << "# username:password_hash\n";
  for (const auto& [username, record] : users_) {
    output << username << ":" << record.password_hash << "\n";
  }

  return ValidationResult::success("User database saved.");
}

void UserStore::add_initial_user_if_missing(
    const std::string& username,
    const std::string& password) {
  if (!exists(username)) {
    users_[username] = UserRecord{username, password_hash(username, password)};
  }
}

}  // namespace cruce::server
