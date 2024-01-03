#include "gossip_rpc.h"
#include "data.pb.h"
#include "defer.h"
#include "error.h"
#include "gossip.pb.h"

#include "brpc/channel.h"
#include "brpc/controller.h"

int32_t GossipRpc::QueryDataRange(const std::string &addr, EmptyMessage *req,
                                  QueryRangeResponse *rsp,
                                  google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  brpc::Channel channel;
  if (auto ret = InitRpcChannel(addr, channel); ret != OK) {
    return ret;
  }
  brpc::Controller cntl;
  DataService_Stub stub(&channel);
  stub.QueryDataRange(&cntl, req, rsp, done);
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "query data range failed, code:" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    return PROCESS_RPC_FAILED;
  }
  return OK;
}

int32_t GossipRpc::PushData(const std::string &addr, GossipData *req,
                            EmptyMessage *rsp,
                            google::protobuf::Closure *done) {

  defer d([done] { done->Run(); });
  brpc::Channel channel;
  if (auto ret = InitRpcChannel(addr, channel); ret != OK) {
    return ret;
  }
  brpc::Controller cntl;
  GossipService_Stub stub(&channel);
  stub.PushData(&cntl, req, rsp, done);
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "push data failed, code:" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    return PROCESS_RPC_FAILED;
  }
  return OK;
}

int32_t GossipRpc::PullData(const std::string &addr, GossipData *req,
                            GossipData *rsp, google::protobuf::Closure *done) {

  defer d([done] { done->Run(); });
  brpc::Channel channel;
  if (auto ret = InitRpcChannel(addr, channel); ret != OK) {
    return ret;
  }
  brpc::Controller cntl;
  GossipService_Stub stub(&channel);
  stub.PullData(&cntl, req, rsp, done);
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "pull data failed, code:" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    return PROCESS_RPC_FAILED;
  }
  return OK;
}
