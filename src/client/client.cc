#include "client.h"
#include "error.h"

#include <brpc/controller.h>
#include <cassert>
#include <sstream>

#include "brpc/channel.h"
#include "data.pb.h"

Client::Client(const std::vector<addr_t> &servers,
               const std::vector<node_id_t> &ports) {
  assert(servers.size() == ports.size());
  for (size_t i = 0; i < servers.size(); ++i) {
    node_ips_[ports[i]] = servers[i];
  }
}

int32_t Client::Write(node_id_t id, const Range &data) {
  if (!data.Valid()) {
    return INVALID_PARAM;
  }
  std::string ip = FindIp(id);
  if (ip.empty()) {
    return NO_SUCH_NODE_FOUND;
  }

  int ret = OK;
  brpc::Channel channel;
  brpc::ChannelOptions opt;
  if ((ret = channel.Init(ip.c_str(), &opt)) != 0) {
    LOG(ERROR) << "init channel failed, with code=" << ret;
    return INIT_RPC_CHANNEL_FAILED;
  }
  brpc::Controller cntl;
  WriteDataRequest req;
  WriteDataResponse rsp;
  auto *d = req.add_data();
  d->set_start(data.start);
  d->set_end(data.end);
  DataService_Stub stub(&channel);
  stub.WriteData(&cntl, &req, &rsp, nullptr); // sync call

  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "send write rpc to " << ip
               << " failed, ctnl error code=" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    return cntl.ErrorCode();
  }
  if (rsp.error_code() != OK) {
    LOG(ERROR) << ip << " write data rsp with error code " << rsp.error_code()
               << " msg:" << (rsp.has_error_msg() ? rsp.error_msg() : "null");
    return rsp.error_code();
  }
  return OK;
}

int32_t Client::Read(node_id_t id, std::string *result) {
  std::string ip;
  ip = FindIp(id);
  if (ip.empty()) {
    return NO_SUCH_NODE_FOUND;
  }
  int32_t ret = OK;
  brpc::Channel channel;
  brpc::ChannelOptions opt;
  if ((ret = channel.Init(ip.c_str(), &opt)) != 0) {
    LOG(ERROR) << "init channel failed, with code=" << ret;
    return INIT_RPC_CHANNEL_FAILED;
  }
  brpc::Controller cntl;
  EmptyMessage req, rsp;
  DataService_Stub stub(&channel);
  stub.QueryDataRange(&cntl, &req, &rsp, nullptr); // sync call
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "send read rpc to " << ip
               << " failed, ctnl error code=" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    return cntl.ErrorCode();
  }
  *result = cntl.response_attachment().to_string();
  return OK;
}

int32_t Client::Clear(node_id_t id) {
  std::string ip;
  ip = FindIp(id);
  if (ip.empty()) {
    return NO_SUCH_NODE_FOUND;
  }
  int32_t ret = OK;
  brpc::Channel channel;
  brpc::ChannelOptions opt;
  if ((ret = channel.Init(ip.c_str(), &opt)) != 0) {
    LOG(ERROR) << "init channel failed, with code=" << ret;
    return INIT_RPC_CHANNEL_FAILED;
  }
  brpc::Controller cntl;
  EmptyMessage req, rsp;
  DataService_Stub stub(&channel);
  stub.ClearData(&cntl, &req, &rsp, nullptr); // sync call
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "send clear rpc to " << ip
               << " failed, ctnl error code=" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    return cntl.ErrorCode();
  }
  return OK;
}

std::string Client::ServerInfo() const {
  std::stringstream ss;
  for (const auto &[id, addr] : node_ips_) {
    ss << "[node id]=" << id << ","
       << "[addr]=" << addr << '\n';
  }
  return ss.str();
}

std::vector<node_id_t> Client::ListAllNodes() const {
  std::vector<node_id_t> nodes;
  nodes.reserve(node_ips_.size());
  for (const auto &[node, addr] : node_ips_) {
    nodes.push_back(node);
  }
  return nodes;
}

std::string Client::FindIp(node_id_t node_id) {
  auto it = node_ips_.find(node_id);
  return it == node_ips_.end() ? "" : it->second;
}
