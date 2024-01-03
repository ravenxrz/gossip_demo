#pragma once

class RangeStorage;

#include "common.h"
#include "data.pb.h"
#include "service.h"
#include "singleton.h"

#include "brpc/server.h"

class Server : public Singleton<Server> {
  SingletonClass(Server);
public:
  Server(addr_t addr);

  int32_t Init();

  void Run();

  void RegisterPeer(const addr_t &peer);

  const std::set<addr_t> &GetPeers() const { return peers_; }

private:
  addr_t self_;
  std::set<addr_t> peers_;

  RangeStorage *storage_;

  brpc::Server rpc_server_;
  DataServiceImpl data_service_;
};
