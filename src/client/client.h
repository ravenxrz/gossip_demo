#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <map>

#include "common.h"
#include "error.h"

class Client {
public:
  using node_id_t = uint32_t;
  using ip_t = std::string;
  // REQUIRED: servers.size() = nodes.size()
  Client(const std::vector<ip_t> &servers, const std::vector<node_id_t> &nodes);

  int32_t Write(node_id_t id, const Range& data);

  int32_t Read(node_id_t id, std::string* result);

  std::string ServerInfo() const;

private:
  std::string FindIp(node_id_t node_id);

  std::map<node_id_t, ip_t> node_ips_;
};
