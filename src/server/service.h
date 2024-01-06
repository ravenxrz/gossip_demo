#pragma once

#include "data.pb.h"
#include "gossip.pb.h"

class RangeStorage;

class DataServiceImpl : public DataService {
 public:
  DataServiceImpl(RangeStorage* storage) : storage_(storage) {}

  void WriteData(google::protobuf::RpcController* controller,
                 const ::WriteDataRequest* request,
                 ::WriteDataResponse* response,
                 ::google::protobuf::Closure* done) override;

  void QueryDataRange(google::protobuf::RpcController* controller,
                      const ::EmptyMessage* request,
                      ::QueryRangeResponse* response,
                      ::google::protobuf::Closure* done) override;

  void ClearData(google::protobuf::RpcController* controller,
                 const ::EmptyMessage* request, ::EmptyMessage* response,
                 ::google::protobuf::Closure* done) override;

 private:
  RangeStorage* storage_;
};

class GossipServiceImpl : public GossipService {
 public:
  GossipServiceImpl(RangeStorage* storage) : storage_(storage) {}

  void ManualGossip(google::protobuf::RpcController* controller,
                    const ::EmptyMessage* request, ::EmptyMessage* response,
                    ::google::protobuf::Closure* done) override;
  void PushData(google::protobuf::RpcController* controller,
                const ::GossipData* request, ::EmptyMessage* response,
                ::google::protobuf::Closure* done) override;
  void PullData(google::protobuf::RpcController* controller,
                const ::GossipData* request, ::GossipData* response,
                ::google::protobuf::Closure* done) override;

 private:
  RangeStorage* storage_;
};
