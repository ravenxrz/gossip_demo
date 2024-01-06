#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "common.h"
#include "error.h"

class Client {
public:
  // REQUIRED: servers.size() = nodes.size()
  Client(const std::vector<addr_t> &servers,
         const std::vector<node_id_t> &nodes);

  int32_t Write(node_id_t id, const Range &data);

  int32_t Read(node_id_t id, std::string *result);

  int32_t Clear(node_id_t id);

  std::string ServerInfo() const;

  std::string FindIp(node_id_t node_id);

  std::vector<node_id_t> ListAllNodes() const;

private:
  std::map<node_id_t, addr_t> node_ips_;
};
