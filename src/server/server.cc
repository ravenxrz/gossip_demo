#include "server.h"
#include "error.h"
#include "storage.h"

#include "glog/logging.h"
#include "server_flags.h"

DECLARE_uint32(port);

Server::Server(addr_t addr)
    : self_(std::move(addr)), storage_(new MemRangeStorage),
      data_service_(storage_) {}

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
  // Start the server.
  brpc::ServerOptions options;
  if (rpc_server_.Start(("localhost:" + std::to_string(FLAGS_port)).c_str(),
                        &options) != 0) {
    LOG(ERROR) << "start server failed";
    return SERVICE_INIT_FAILED;
  }
  LOG(INFO) << "init server end";
  return OK;
}

void Server::Run() { rpc_server_.RunUntilAskedToQuit(); }
