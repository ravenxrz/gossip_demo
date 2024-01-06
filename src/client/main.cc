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

#include "config_parser.h"
#include "op_cntl.h"
#include "string_utils.h"

#include "butil/string_splitter.h"
#include "butil/strings/string_piece.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DECLARE_int32(FLAGS_minloglevel);
DECLARE_string(log_dir);
DECLARE_int32(stderrthreshold);

DEFINE_string(servers, "127.0.0.1:4444,127.0.0.1:5555",
              "server addrs, split by ','");
DEFINE_string(op_file, "", "operation file");
DEFINE_string(conf_file, "", "cluster config file");

std::unique_ptr<Client> g_client;
std::unique_ptr<OpCntl> g_op_cntl;

void InitLogSystem(char** argv) {
  FLAGS_log_dir = "./client_log";
  FLAGS_minloglevel = google::INFO;
  FLAGS_stderrthreshold = google::INFO;
  google::InitGoogleLogging(argv[0]);
}

void InitConf() {
  ConfigParser::GetInstance().Init(FLAGS_conf_file);
  // set flags
  std::string value;
  if (ConfigParser::GetInstance().Get("servers", &value)) {
    FLAGS_servers = value;
  }
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
  g_op_cntl.reset(new OpCntl(g_client.get()));
}

void InputLoop() {
  enum OpType : uint32_t {
    Min = 0,
    List = 0,
    Write = 1,
    Read = 2,
    Clear = 3,
    NUM = 4
  };
  uint32_t operation = List;
  int64_t node = -1;
  Range write_data;
  std::cerr << "###############################################\n"
               "                  SDK Console                  \n"
               "###############################################\n";
  while (true) {
    std::cerr << "input opeartion:\n"
                 "0. list servers\n"
                 "1. write\n"
                 "2. read\n"
                 "3. clear\n";
    std::cin >> operation;
    if (operation < Min || operation >= NUM) {
      std::cerr << "invalid operation number\n";
      continue;
    } else if (operation == List) {
      std::cerr << g_client->ServerInfo();
      continue;
    } else if (operation == Write) {
      std::cerr << "input write range(format: start,end):\n";
      std::cin >> write_data;
    }
    std::cerr << "choose node id(-1 is for all nodes):\n"
              << g_client->ServerInfo();
    std::cin >> node;
    if (operation == Write) {
      g_op_cntl->Write(node, write_data);
    } else if (operation == Read) {
      g_op_cntl->Read(node);
    } else if (operation == Clear) {
      g_op_cntl->Clear(node);
    }
  }
}

void HandleWritline(const std::string& line) {
  std::vector<std::string> pattern;
  SplitString(line, ':', &pattern);
  if (pattern.size() != 3) {
    LOG(ERROR) << "invalid write pattern(eg: write:node:start,end):" << line;
    return;
  }
  char* str_end = nullptr;
  auto node_id = std::strtoll(pattern[1].c_str(), &str_end, 10);
  std::vector<std::string> num;
  SplitString(pattern[2], ',', &num);
  if (num.size() != 2) {
    LOG(ERROR) << "invalid write pattern(eg: write:node:range):" << line;
    return;
  }
  Range write_data;
  write_data.start = std::strtoul(num[0].c_str(), &str_end, 10);
  write_data.end = std::strtoul(num[1].c_str(), &str_end, 10);
  g_op_cntl->Write(node_id, write_data);
}

void HandleReadLine(const std::string& line) {
  std::vector<std::string> pattern;
  SplitString(line, ':', &pattern);
  if (pattern.size() != 2) {
    LOG(ERROR) << "invalid read pattern: " << line;
    return;
  }
  std::string read_result;
  char* str_end = nullptr;
  auto node_id = std::strtoll(pattern[1].c_str(), &str_end, 10);
  g_op_cntl->Read(node_id);
}

void HandleClearline(const std::string& line) {
  std::vector<std::string> pattern;
  SplitString(line, ':', &pattern);
  if (pattern.size() != 2) {
    LOG(ERROR) << "invalid read pattern: " << line;
    return;
  }
  char* str_end = nullptr;
  auto node_id = std::strtoll(pattern[1].c_str(), &str_end, 10);
  g_op_cntl->Clear(node_id);
}

// file format:
// write:node_id:start,end
// read:node_id
// clear:node_id
void ParseOpFile() {
  std::ifstream in(FLAGS_op_file, std::ios_base::in);
  if (!in.is_open()) {
    LOG(ERROR) << "open " << FLAGS_op_file << " failed"
               << " state:" << in.rdstate();
    return;
  }
  std::string line;
  while (in >> line) {
    LOG(INFO) << "line content: " << line;
    StrTrim(&line);
    try {
      if (StrStartsWith(line, "write")) {
        HandleWritline(line);
      } else if (StrStartsWith(line, "read")) {
        HandleReadLine(line);
      } else if (StrStartsWith(line, "clear")) {
        HandleClearline(line);
      } else {
        LOG(ERROR) << "invalid op line:" << line;
      }
    } catch (...) { LOG(ERROR) << "parse file error, with line:" << line; }
  }
}

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  InitLogSystem(argv);
  InitConf();
  InitClient();
  if (FLAGS_op_file.empty()) {
    InputLoop();
  } else {
    ParseOpFile();
  }
  return 0;
}
