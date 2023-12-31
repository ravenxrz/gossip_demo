#pragma once

#include "data.pb.h"

class RangeStorage;

class DataServiceImpl : public DataService {
public:
  DataServiceImpl(RangeStorage *storage) : storage_(storage) {}

  void WriteData(google::protobuf::RpcController *controller,
                 const ::WriteDataRequest *request,
                 ::WriteDataResponse *response,
                 ::google::protobuf::Closure *done) override;

  void QueryDataRange(google::protobuf::RpcController *controller,
                      const ::EmptyMessage *request, ::EmptyMessage *response,
                      ::google::protobuf::Closure *done) override;

private:
  RangeStorage *storage_;
};
