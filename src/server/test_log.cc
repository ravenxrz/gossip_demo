#include "glog/logging.h"

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_minloglevel = google::INFO;
  FLAGS_log_dir = "./log";
  LOG(INFO) << "INFO";
  LOG(WARNING) << "WARNING";
  LOG(ERROR) << "ERROR";
  // LOG(FATAL) << "FATAL";
  return 0;
}
