/**
 * Client tool:
 * 1. write data to choosed servers
 * 2. read data from choosed servers
 */

#include "client.h"

#include <butil/strings/string_piece.h>
#include <memory>

#include "butil/string_splitter.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DECLARE_int32(FLAGS_minloglevel);
DECLARE_string(log_dir);
DECLARE_int32(stderrthreshold);

DEFINE_string(servers, "127.0.0.1:5555", "server addrs, split by ','");

std::unique_ptr<Client> g_client;

void init_log_system(char **argv) {
  FLAGS_log_dir = "./client_log";
  FLAGS_minloglevel = google::INFO;
  FLAGS_stderrthreshold = google::INFO;
}

void init_client() {
  std::vector<std::string> addrs;
  std::vector<uint32_t> node_ids;
  uint32_t i = 0;
  butil::StringSplitter splitter(butil::StringPiece(FLAGS_servers), ',');
  while (splitter) {
    addrs.emplace_back(splitter.field(), splitter.length());
    node_ids.push_back(i++);
    splitter++;
  }
  g_client.reset(new Client(addrs, node_ids));
  LOG(INFO) << g_client->ServerInfo();
}

void write() { g_client->Write(0, {1, 100}); }

int main(int argc, char *argv[]) {
  init_log_system(argv);
  init_client();
  write();
  return 0;
}
