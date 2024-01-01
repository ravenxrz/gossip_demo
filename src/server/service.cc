#include "service.h"

#include <sstream>

#include "brpc/controller.h"
#include "data.pb.h"
#include "defer.h"
#include "error.h"
#include "gossip.pb.h"
#include "storage.h"

void DataServiceImpl::WriteData(google::protobuf::RpcController *controller,
                                const ::WriteDataRequest *request,
                                ::WriteDataResponse *response,
                                ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  for (uint32_t i = 0; i < request->data_size(); ++i) {
    storage_->Write(Range(request->data(i).start(), request->data(i).end()));
  }
  response->set_error_code(OK);
}

void DataServiceImpl::QueryDataRange(
    google::protobuf::RpcController *controller, const ::EmptyMessage *request,
    ::QueryRangeResponse *response, ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  const auto &ranges = storage_->Read();
  for (const auto &r : ranges) {
    auto *d = response->add_ranges();
    d->set_start(r.start);
    d->set_end(r.end);
  }
  // brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
  // cntl->response_attachment().append(ss.str());
}

void DataServiceImpl::ClearData(google::protobuf::RpcController *controller,
                                const ::EmptyMessage *request,
                                ::EmptyMessage *response,
                                ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  storage_->Clear();
}

void GossipServiceImpl::ManualGossip(
    google::protobuf::RpcController *controller, const ::EmptyMessage *request,
    ::EmptyMessage *response, ::google::protobuf::Closure *done) {}

void GossipServiceImpl::PushData(google::protobuf::RpcController *controller,
                                 const ::GossipData *request,
                                 ::EmptyMessage *response,
                                 ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  // TODO(zhangxingrui): async
  for (uint32_t i = 0; i < request->ranges_size(); ++i) {
    storage_->Write({request->ranges(i).start(), request->ranges(i).end()});
  }
}

void GossipServiceImpl::PullData(google::protobuf::RpcController *controller,
                                 const ::GossipData *request,
                                 ::GossipData *response,
                                 ::google::protobuf::Closure *done) {
  defer d([done] { done->Run(); });
  // Now that we take ranges as meta and data
  // just copy request to response and return
  for (uint32_t i = 0; i < request->ranges_size(); ++i) {
    auto *d = response->add_ranges();
    d->set_start(request->ranges(i).start());
    d->set_end(request->ranges(i).end());
  }
}
