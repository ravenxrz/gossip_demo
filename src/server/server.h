#pragma once

#include "gossip_rpc.h"
class RangeStorage;

#include "common.h"
#include "data.pb.h"
#include "service.h"
#include "singleton.h"
#include "timer.h"
#include "worker.h"

#include "brpc/server.h"

class Server : public Singleton<Server> {
  SingletonClass(Server);
  Server();

 public:
  ~Server();

  int32_t Init();

  void Run();

  void SetAddr(addr_t addr) { self_ = std::move(addr); }

  addr_t GetAddr() const { return self_; }

  void RegisterPeer(const addr_t& peer);

  const std::set<addr_t>& GetPeers() const { return peers_; }

 private:
  void StartGossip();

  addr_t self_;
  std::set<addr_t> peers_;

  TaskWorker worker_;
  Timer* timer_{nullptr};
  RangeStorage* storage_{nullptr};
  brpc::Server rpc_server_;
  DataServiceImpl data_service_;
  GossipServiceImpl gossip_service_;
};
