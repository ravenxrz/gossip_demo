/**
 * Client tool:
 * 1. write data to choosed servers
 * 2. read data from choosed servers
 */

#include "client.h"

#include <cstdlib>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>

#include "butil/string_splitter.h"
#include "butil/strings/string_piece.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "string_utils.h"

DECLARE_int32(FLAGS_minloglevel);
DECLARE_string(log_dir);
DECLARE_int32(stderrthreshold);

DEFINE_string(servers, "127.0.0.1:4444,127.0.0.1:5555",
              "server addrs, split by ','");
DEFINE_string(op_file, "", "operation file");

std::unique_ptr<Client> g_client;

void InitLogSystem(char **argv) {
  FLAGS_log_dir = "./client_log";
  FLAGS_minloglevel = google::INFO;
  FLAGS_stderrthreshold = google::INFO;
}

void InitClient() {
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
  LOG(INFO) << "server info:\n" << g_client->ServerInfo();
}

void InputLoop() {
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

void HandleWritline(const std::string &line) {
  std::vector<std::string> pattern;
  SplitString(line, ':', &pattern);
  if (pattern.size() != 3) {
    LOG(ERROR) << "invalid write pattern(eg: write:node:start,end):" << line;
    return;
  }
  char *str_end = nullptr;
  auto node_id = std::strtoll(pattern[1].c_str(), &str_end, 10);
  int32_t ret = OK;
  std::vector<std::string> num;
  SplitString(pattern[2], ',', &num);
  if (num.size() != 2) {
    LOG(ERROR) << "invalid write pattern(eg: write:node:range):" << line;
    return;
  }
  Range write_data;
  write_data.start = std::strtoul(num[0].c_str(), &str_end, 10);
  write_data.end = std::strtoul(num[1].c_str(), &str_end, 10);
  if (node_id == -1) {
    for (auto node : g_client->ListAllNodes()) {
      ret = g_client->Write(node, write_data);
      if (ret != OK) {
        LOG(ERROR) << "write to node:" << node << " failed, code:" << ret;
      }
    }
  } else {
    ret = g_client->Write(node_id, write_data);
    if (ret != OK) {
      LOG(ERROR) << "write to node:" << node_id << " failed, code:" << ret;
    }
  }
}

void HandleReadLine(const std::string &line) {
  std::vector<std::string> pattern;
  SplitString(line, ':', &pattern);
  if (pattern.size() != 2) {
    LOG(ERROR) << "invalid read pattern: " << line;
    return;
  }
  int32_t ret = OK;
  std::string read_result;
  char *str_end = nullptr;
  auto node_id = std::strtoll(pattern[1].c_str(), &str_end, 10);
  if (node_id == -1) {
    for (auto node : g_client->ListAllNodes()) {
      ret = g_client->Read(node, &read_result);
      if (ret != OK) {
        LOG(ERROR) << "read from node:" << node << " failed, code:" << ret;
      } else {
        LOG(INFO) << "read node:" << node << " data:\n" << read_result;
      }
    }
  } else {
    ret = g_client->Read(node_id, &read_result);
    if (ret != OK) {
      LOG(ERROR) << "read from node:" << node_id << " failed, code:" << ret;
    } else {
      LOG(INFO) << "read node:" << node_id << " data:\n" << read_result;
    }
  }
}

// file format:
// write:node_id:start,end
// read:node_id
void ParseOpFile() {
  std::ifstream in(FLAGS_op_file, std::ios_base::in);
  if (!in.is_open()) {
    LOG(ERROR) << "open " << FLAGS_op_file << " failed"
               << " state:" << in.rdstate();
    return;
  }
  std::string line;
  int64_t node_id = -1;
  while (in >> line) {
    LOG(INFO) << "line content: " << line;
    StrTrim(&line);
    try {
      if (StrStartsWith(line, "write")) {
        HandleWritline(line);
      } else if (StrStartsWith(line, "read")) {
        HandleReadLine(line);
      } else {
        LOG(ERROR) << "invalid op line:" << line;
      }
    } catch (...) {
      LOG(ERROR) << "parse file error, with line:" << line;
    }
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  InitLogSystem(argv);
  InitClient();
  if (FLAGS_op_file.empty()) {
    InputLoop();
  } else {
    ParseOpFile();
  }
  return 0;
}
