#include "server.h"
#include "error.h"
#include "gossip_rpc.h"
#include "gossip_task.h"
#include "service.h"
#include "storage.h"

#include "glog/logging.h"
#include "server_flags.h"

DECLARE_uint32(port);
DECLARE_uint32(gossip_interval_ms);

Server::Server()
    : storage_(new MemRangeStorage), data_service_(storage_),
      gossip_service_(storage_) {}

Server::~Server() { delete timer_; }

void Server::RegisterPeer(const addr_t &peer) {
  if (peer == self_) {
    LOG(ERROR) << "can't register myself as peer";
    return;
  }
  auto ret = peers_.insert(peer);
  if (!ret.second) {
    LOG(ERROR) << "insert " << peer << " failed";
    return;
  }
  LOG(INFO) << "register " << peer;
}

int32_t Server::Init() {
  LOG(INFO) << "init server start";
  if (rpc_server_.AddService(&data_service_, brpc::SERVER_OWNS_SERVICE) != 0) {
    LOG(ERROR) << "add data service failed";
    return SERVICE_INIT_FAILED;
  }
  if (rpc_server_.AddService(&gossip_service_, brpc::SERVER_OWNS_SERVICE) !=
      0) {
    LOG(ERROR) << "add gossip service failed";
    return SERVICE_INIT_FAILED;
  }
  // Start the server.
  brpc::ServerOptions options;
  if (auto ret = rpc_server_.Start(
          ("localhost:" + std::to_string(FLAGS_port)).c_str(), &options);
      ret != 0) {
    LOG(ERROR) << "start server failed:" << ret;
    return SERVICE_INIT_FAILED;
  }
  LOG(INFO) << "init server end";
  return OK;
}

void Server::StartGossip() {
  for (auto peer : peers_) {
    timer_->Reigster(FLAGS_gossip_interval_ms, [this, peer](timer_task_id_t) {
      (new GossipTask(peer, storage_, &GossipRpc::GetInstance()))->Run();
    });
  }
}

void Server::Run() {
  worker_.Start();
  timer_ = new Timer(&worker_);
  StartGossip();
  rpc_server_.RunUntilAskedToQuit();
}
