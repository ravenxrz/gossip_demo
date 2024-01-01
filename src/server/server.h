#pragma once

class RangeStorage;

#include "brpc/server.h"
#include "data.pb.h"
#include "service.h"

class Server {
public:
  Server();

  int32_t Init();

  void Run();

private:
  RangeStorage *storage_;

  brpc::Server rpc_server_;
  DataServiceImpl data_service_;
};
