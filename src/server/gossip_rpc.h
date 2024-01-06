#pragma once

#include "igossip_rpc.h"

class GossipRpc : public IGossipRpc {
 public:
  static GossipRpc& GetInstance() {
    static GossipRpc rpc;
    return rpc;
  }

  int32_t QueryDataRange(const std::string& addr, EmptyMessage* req,
                         QueryRangeResponse* rsp,
                         google::protobuf::Closure* done) override;

  int32_t PushData(const std::string& addr, GossipData* req, EmptyMessage* rsp,
                   google::protobuf::Closure* done) override;

  int32_t PullData(const std::string& addr, GossipData* req, GossipData* rsp,
                   google::protobuf::Closure* done) override;
};