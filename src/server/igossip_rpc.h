#pragma once

#include "brpc/channel.h"
#include "data.pb.h"
#include "error.h"
#include "google/protobuf/stubs/callback.h"
#include "gossip.pb.h"

class IGossipRpc {
public:
  virtual ~IGossipRpc() = default;

  // Query `addr` node data ranges
  virtual int32_t QueryDataRange(const std::string &addr, EmptyMessage *req,
                                 QueryRangeResponse *rsp,
                                 google::protobuf::Closure *done) = 0;

  virtual int32_t PushData(const std::string &addr, GossipData *req,
                           EmptyMessage *rsp,
                           google::protobuf::Closure *done) = 0;

  virtual int32_t PullData(const std::string &addr, GossipData *req,
                           GossipData *rsp,
                           google::protobuf::Closure *done) = 0;

protected:
  int32_t InitRpcChannel(const std::string &addr, brpc::Channel &channel) {
    brpc::ChannelOptions opt;
    int32_t ret = 0;
    if ((ret = channel.Init(addr.c_str(), &opt)) != 0) {
      LOG(ERROR) << "init channel failed:" << ret;
      return INIT_RPC_CHANNEL_FAILED;
    }
	return OK;	
  }
};