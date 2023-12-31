#include "server.h"
#include "error.h"
#include "storage.h"

#include "glog/logging.h"

DEFINE_uint32(port, 5555, "server port");

Server::Server() : storage_(new MemRangeStorage), data_service_(storage_) {}

int32_t Server::init() {
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

void Server::run() { rpc_server_.RunUntilAskedToQuit(); }
