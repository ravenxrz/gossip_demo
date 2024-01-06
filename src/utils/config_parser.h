#pragma once

#include "singleton.h"

#include <mutex>
#include <string>
#include <unordered_map>

class ConfigParser : public Singleton<ConfigParser> {
  SingletonClass(ConfigParser);

 public:
  ConfigParser(const ConfigParser& rhs) = delete;
  ConfigParser(ConfigParser&& rhs) = delete;
  ConfigParser& operator=(const ConfigParser& rhs) = delete;
  ConfigParser& operator=(ConfigParser&& rhs) = delete;

  void Init(const std::string& conf_path);

  bool Get(const std::string& flag_name, std::string* flag_value);

 private:
  ConfigParser() = default;
  static ConfigParser* parser_;
  std::unordered_map<std::string, std::string> conf_map_;
};
