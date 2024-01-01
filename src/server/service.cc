#include "service.h"

#include <sstream>

#include "brpc/controller.h"
#include "defer.h"
#include "error.h"
#include "storage.h"

void DataServiceImpl::WriteData(google::protobuf::RpcController *controller,
                                const ::WriteDataRequest *request,
                                ::WriteDataResponse *response,
                                ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  storage_->Write(Range(request->start(), request->end()));
  response->set_error_code(OK);
}

void DataServiceImpl::QueryDataRange(
    google::protobuf::RpcController *controller, const ::EmptyMessage *request,
    ::EmptyMessage *response, ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  const auto &ranges = storage_->Read();
  std::stringstream ss;
  for (const auto &r : ranges) {
    ss << r.ToString() << '\n';
  }
  brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
  cntl->response_attachment().append(ss.str());
}

void DataServiceImpl::ClearData(google::protobuf::RpcController *controller,
                                const ::EmptyMessage *request,
                                ::EmptyMessage *response,
                                ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  storage_->Clear();
}
