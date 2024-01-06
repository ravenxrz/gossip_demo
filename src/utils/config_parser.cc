#include "config_parser.h"

#include <fstream>
#include <mutex>

#include "glog/logging.h"
#include "string_utils.h"

void ConfigParser::Init(const std::string& conf_path) {
  if (conf_path.empty()) {
    LOG(ERROR) << "conf file is empty, use default flags";
    return;
  }
  std::ifstream in(conf_path);
  if (!in.is_open()) {
    LOG(ERROR) << "open " << conf_path << " failed";
    return;
  }
  std::string line;
  std::vector<std::string> sp;
  while (in >> line) {
    StrTrim(&line);
    SplitString(line, '=', &sp);
    if (sp.size() != 2) {
      LOG(ERROR) << "parse " << line << " failed";
      continue;
    }
    StrTrim(&sp[0]);
    StrTrim(&sp[1]);
    conf_map_.insert({std::move(sp[0]), std::move(sp[1])});
  }
}

bool ConfigParser::Get(const std::string& flag_name, std::string* flag_value) {
  if (auto it = conf_map_.find(flag_name); it != conf_map_.end()) {
    *flag_value = it->second;
    return true;
  }
  return false;
}
