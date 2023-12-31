#include "error.h"
#include "service.h"

#include "brpc/server.h"
#include "gflags/gflags.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include <glog/logging.h>

DEFINE_uint32(port, 5555, "server port");
DEFINE_uint32(log_level, 0, "0:trace, 1:debug, 2:info, 3:warn, 4:error");
DEFINE_string(log_path, "./simple_gossip.log", "log path");

brpc::Server g_server;

int32_t init_server() {
  LOG(INFO) << "init server start";
  DataServiceImpl data_service;
  if (g_server.AddService(&data_service, brpc::SERVER_DOESNT_OWN_SERVICE) !=
      0) {
    LOG(ERROR) << "add data service failed";
    return SERVICE_INIT_FAILED;
  }
  // Start the server.
  brpc::ServerOptions options;
  if (g_server.Start(("localhost:" + std::to_string(FLAGS_port)).c_str(),
                     &options) != 0) {
    LOG(ERROR) << "start server failed";
    return SERVICE_INIT_FAILED;
  }
  LOG(INFO) << "init server end";
  return OK;
}

void init_log_system(char **argv) { google::InitGoogleLogging(argv[0]); }

void init(int argc, char **argv) {
  init_log_system(argv);
  int32_t ret = init_server();
  assert(ret == OK);
}

void run() {
  // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
  LOG(INFO) << "run server, user [ctrl -c] to exit";
  g_server.RunUntilAskedToQuit();
}

int main(int argc, char *argv[]) {
  // Parse gflags. We recommend you to use gflags as well.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  init(argc, argv);
  run();
  return 0;
}
