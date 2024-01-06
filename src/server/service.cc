#include "service.h"

#include <sstream>

#include "brpc/controller.h"
#include "data.pb.h"
#include "defer.h"
#include "error.h"
#include "gossip.pb.h"
#include "gossip_rpc.h"
#include "gossip_task.h"
#include "server.h"
#include "storage.h"

// extern std::unique_ptr<Server> g_server;

void DataServiceImpl::WriteData(google::protobuf::RpcController *controller,
                                const ::WriteDataRequest *request,
                                ::WriteDataResponse *response,
                                ::google::protobuf::Closure *done) {
  defer d([done] {
    if (done != nullptr) {
      done->Run();
    }
  });
  for (int i = 0; i < request->data_size(); ++i) {
    storage_->Write(Range(request->data(i).start(), request->data(i).end()));
  }
  response->set_error_code(OK);
}

void DataServiceImpl::QueryDataRange(
    google::protobuf::RpcController *controller, const ::EmptyMessage *request,
    ::QueryRangeResponse *response, ::google::protobuf::Closure *done) {
  defer d([done] {
    if (done != nullptr) {
      done->Run();
    }
  });
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
  defer d([done] {
    if (done != nullptr) {
      done->Run();
    }
  });
  storage_->Clear();
}

void GossipServiceImpl::ManualGossip(
    google::protobuf::RpcController *controller, const ::EmptyMessage *request,
    ::EmptyMessage *response, ::google::protobuf::Closure *done) {
  defer d([done] {
    if (done != nullptr) {
      done->Run();
    }
  });
  // TODO(zhangxingrui): use thread pool + async
  // manual gossip with all peers
  // GossipTask gossip_task;
  // for (auto peer : g_server->GetPeers()) {
  //   GossipTask gossip_task(peer, storage_, &GossipRpc::GetInstance());
  //   gossip_task.Run();
  // }
}

void GossipServiceImpl::PushData(google::protobuf::RpcController *controller,
                                 const ::GossipData *request,
                                 ::EmptyMessage *response,
                                 ::google::protobuf::Closure *done) {
  defer d([done] {
    if (done != nullptr) {
      done->Run();
    }
  });
  // TODO(zhangxingrui): async
  // DLOG(INFO) << "push request, receive from: "
  //            << ((brpc::Controller *)(controller))->remote_side();
  DLOG(INFO) << "push request, receive from: " << request->addr();
  for (int i = 0; i < request->ranges_size(); ++i) {
    storage_->Write({request->ranges(i).start(), request->ranges(i).end()});
  }
}

void GossipServiceImpl::PullData(google::protobuf::RpcController *controller,
                                 const ::GossipData *request,
                                 ::GossipData *response,
                                 ::google::protobuf::Closure *done) {
  defer d([done] {
    if (done != nullptr) {
      done->Run();
    }
  });
  DLOG(INFO) << "pull request, receive from: " << request->addr();
  // Now that we take ranges as meta and data
  // just copy request to response and return
  for (int i = 0; i < request->ranges_size(); ++i) {
    auto *d = response->add_ranges();
    d->set_start(request->ranges(i).start());
    d->set_end(request->ranges(i).end());
  }
}
