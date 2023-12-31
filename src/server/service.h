#pragma once

#include "data.pb.h"

class DataServiceImpl : public DataService {
public:
  void WriteData(google::protobuf::RpcController *controller,
                 const ::WriteDataRequest *request,
                 ::WriteDataResponse *response,
                 ::google::protobuf::Closure *done) override;
};
