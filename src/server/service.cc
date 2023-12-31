#include "service.h"

void DataServiceImpl::WriteData(google::protobuf::RpcController *controller,
                                const ::WriteDataRequest *request,
                                ::WriteDataResponse *response,
                                ::google::protobuf::Closure *done) {
    done->Run();
}
