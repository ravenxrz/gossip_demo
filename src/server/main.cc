#include "config_parser.h"
#include "error.h"
#include "server.h"
#include "service.h"
#include "string_utils.h"

#include <cstdlib>
#include "brpc/server.h"
#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"
#include "glog/logging.h"

DECLARE_int32(FLAGS_minloglevel);
DECLARE_string(log_dir);
DECLARE_int32(stderrthreshold);
DECLARE_uint32(port);
DECLARE_string(conf_path);

Server* g_server;

void InitLogSystem(char** argv) {
  FLAGS_log_dir = "./server_log";
  FLAGS_minloglevel = google::INFO;
  FLAGS_stderrthreshold = google::INFO;
}

int32_t Init(int argc, char** argv) {
  InitLogSystem(argv);
  ConfigParser::GetInstance().Init(FLAGS_conf_path);

  // regist peers
  std::string self = "localhost:" + std::to_string(FLAGS_port);
  g_server = &(Server::GetInstance());
  g_server->SetAddr(self);
  std::string servers;
  if (ConfigParser::GetInstance().Get("servers", &servers)) {
    LOG(INFO) << "get all servers info:" << servers;
    std::vector<std::string> svrs;
    SplitString(servers, ',', &svrs);
    for (const auto& svr : svrs) {
      if (svr == self) {
        continue;
      }
      g_server->RegisterPeer(svr);
    }
  }
  if (auto ret = g_server->Init(); ret != OK) {
    return ret;
  }
  return OK;
}

void Run() {
  // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
  LOG(INFO) << "run server, press [ctrl -c] to exit";
  g_server->Run();  // block
}

int main(int argc, char* argv[]) {
  // Parse gflags. We recommend you to use gflags as well.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (Init(argc, argv) != OK) {
    LOG(ERROR) << "init failed, exit now";
    return -1;
  }
  Run();
  return 0;
}
