/**
 * Client tool:
 * 1. write data to choosed servers
 * 2. read data from choosed servers
 */

#include "client.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>

#include "butil/string_splitter.h"
#include "butil/strings/string_piece.h"
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

void write() {
  uint32_t start = rand() % 10000;
  uint32_t end = rand() % 10000;
  g_client->Write(0, {std::min(start, end), std::max(start, end)});
}

void run() {
  enum OpType : uint32_t { Write = 0, Read = 1, NUM = 2 };
  uint32_t operation = Write;
  int64_t node = -1;
  Range write_data;
  std::string read_result;
  int32_t ret = OK;
  std::function<int32_t(uint32_t)> call_handle[NUM] = {
      [&write_data](uint32_t node_id) -> int32_t {
        return g_client->Write(node_id, write_data);
      },
      [&read_result](uint32_t node_id) -> int32_t {
        return g_client->Read(node_id, &read_result);
      }};
  std::function<void(int32_t, uint32_t)> ret_handle[NUM] = {
      [](int32_t ret, uint32_t node) {
        if (ret != OK) {
          LOG(ERROR) << "write to node:" << node << " failed, code:" << ret;
        }
      },
      [&read_result](int32_t ret, uint32_t node) {
        if (ret != OK) {
          LOG(ERROR) << "read from node:" << node << " failed, code:" << ret;
        } else {
          std::cout << "read from node:" << node << " data:\n" << read_result;
        }
      }};
  std::cerr << "###############################################\n"
               "                  SDK Console                  \n"
               "###############################################\n";
  while (true) {
    std::cerr << "input opeartion:\n"
                 "0. write\n"
                 "1. read\n";
    std::cin >> operation;
    if (OpType(operation) != Write && OpType(operation) != Read) {
      std::cerr << "invalid operation number\n";
      continue;
    } else if (operation == Write) {
      std::cerr << "input write range(format: start,end):\n";
      std::cin >> write_data;
    }

  chose_node:
    std::cerr << "choose node id(-1 is for all nodes):\n"
              << g_client->ServerInfo();
    std::cin >> node;
    if (node != -1 && g_client->FindIp(uint32_t(node)).empty()) {
      goto chose_node;
    } else if (node != -1) {
      ret = call_handle[operation](node);
      ret_handle[operation](ret, node);
    } else {
      auto nodes = g_client->ListAllNodes();
      for (auto node : nodes) {
        ret = call_handle[operation](node);
        ret_handle[operation](ret, node);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  init_log_system(argv);
  init_client();
  run();
  return 0;
}
