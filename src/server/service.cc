#include "service.h"

#include "error.h"
#include "storage.h"
#include "defer.h"

void DataServiceImpl::WriteData(google::protobuf::RpcController *controller,
                                const ::WriteDataRequest *request,
                                ::WriteDataResponse *response,
                                ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  storage_->Write(Range(request->start(), request->end()));
  response->set_error_code(OK);
}
