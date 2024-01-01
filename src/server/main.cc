#include "error.h"
#include "server.h"
#include "service.h"

#include "brpc/server.h"
#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"
#include "glog/logging.h"

DECLARE_int32(FLAGS_minloglevel);
DECLARE_string(log_dir);
DECLARE_int32(stderrthreshold);

Server g_server;

void InitLogSystem(char **argv) {
  FLAGS_log_dir = "./server_log";
  FLAGS_minloglevel = google::INFO;
  FLAGS_stderrthreshold = google::INFO;
}

void Init(int argc, char **argv) {
  InitLogSystem(argv);
  g_server.Init();
}

void InputLoop() {
  // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
  LOG(INFO) << "run server, press [ctrl -c] to exit";
  g_server.Run(); // block
}

int main(int argc, char *argv[]) {
  // Parse gflags. We recommend you to use gflags as well.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  Init(argc, argv);
  InputLoop();
  return 0;
}
