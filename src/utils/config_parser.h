#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

class ConfigParser {
public:
  static ConfigParser &GetInstance() {
    if (parser_ == nullptr) {
      std::lock_guard<std::mutex> guard(mu_);
      if (parser_ == nullptr) {
        parser_ = new ConfigParser();
      }
    }
    return *parser_;
  }

  ConfigParser(const ConfigParser &rhs) = delete;
  ConfigParser(ConfigParser &&rhs) = delete;
  ConfigParser &operator=(const ConfigParser &rhs) = delete;
  ConfigParser &operator=(ConfigParser &&rhs) = delete;

  void Init(const std::string &conf_path);

  bool Get(const std::string &flag_name, std::string *flag_value);

private:
  ConfigParser() = default;
  static ConfigParser *parser_;
  static std::mutex mu_;
  std::unordered_map<std::string, std::string> conf_map_;
};
