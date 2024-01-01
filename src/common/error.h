#pragma once

#include <cstdint>

enum ErrorCode : int32_t {
  OK = 0,
  INVALID_PARAM = 1,
  INIT_RPC_CHANNEL_FAILED = 2,
  PROCESS_RPC_FAILED = 3,

  // client
  NO_SUCH_NODE_FOUND = 1000,
  
  // server 
  SERVICE_INIT_FAILED = 2000,
};
