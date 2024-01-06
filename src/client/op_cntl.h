#pragma once

#include "client.h"
#include "common.h"

#include <functional>

class OpCntl {
 public:
  OpCntl(Client* client) : client_(client) {}

  void Write(node_id_t node, const Range& w);

  void Read(node_id_t node);

  void Clear(node_id_t node);

 private:
  Client* client_;
};
