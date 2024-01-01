#include "gossip_task.h"

#include "brpc/channel.h"
#include "data.pb.h"
#include "error.h"
#include "gossip.pb.h"

void GossipTask::Run() {
loop:
  switch (GetTaskState()) {
  case QUERY_DATA_RANGE:
    SetTaskState(READ_LOCAL_RANGE);
    QueryRange();
    goto loop;
  case READ_LOCAL_RANGE:
    SetTaskState(PULL_DATA);
    ReadLocalRange();
    goto loop;
  case DIFF_RANGE:
    SetTaskState(PULL_DATA);
    Diff();
    goto loop;
  case PULL_DATA:
    SetTaskState(PUSH_DATA);
    PullData();
    goto loop;
  case PUSH_DATA:
    SetTaskState(TASK_FIN);
    PushData();
    goto loop;
  case TASK_FIN:
  default:
    break;
  }
}

void GossipTask::QueryRange() {
  brpc::Channel channel;
  brpc::ChannelOptions opt;
  int32_t ret = 0;
  if ((ret = channel.Init(peer_.c_str(), &opt)) != 0) {
    LOG(ERROR) << "init channel failed:" << ret;
    SetTaskStatus(INIT_RPC_CHANNEL_FAILED);
    SetTaskState(TASK_FIN);
    return;
  }
  brpc::Controller cntl;
  EmptyMessage req;
  QueryRangeResponse rsp;
  DataService_Stub stub(&channel);
  // TODO(zhangxingrui): async
  stub.QueryDataRange(&cntl, &req, &rsp, nullptr); // sync call
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "query data range failed, code:" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    SetTaskStatus(PROCESS_RPC_FAILED);
    SetTaskState(TASK_FIN);
    return;
  }
  for (uint32_t i = 0; i < rsp.ranges_size(); ++i) {
    peer_ranges_.emplace_back(rsp.ranges(i).start(), rsp.ranges(i).end());
  }
}

void GossipTask::ReadLocalRange() {
  const auto &ranges = storage_->Read();
  for (const auto &r : ranges) {
    self_ranges_.push_back(r);
  }
}

void GossipTask::Diff() {
  int i = 0;
  int j = 0;
  while (i < self_ranges_.size() && j < peer_ranges_.size()) {
    if (self_ranges_[i].end <= peer_ranges_[j].start) {
      need_push_.push_back(self_ranges_[i++]);
    } else if (self_ranges_[i].start >= peer_ranges_[j].end) {
      need_pull_.push_back(peer_ranges_[j++]);
    } else {
      assert(Range::IsOverlap(self_ranges_[i], peer_ranges_[j]));
      if (self_ranges_[i].start < peer_ranges_[j].start &&
          self_ranges_[i].end < peer_ranges_[j].end) {
        need_push_.emplace_back(self_ranges_[i].start, peer_ranges_[j].start);
        need_pull_.emplace_back(self_ranges_[i].end, peer_ranges_[j].end);
        i++;
      } else if (self_ranges_[i].start > peer_ranges_[j].start &&
                 self_ranges_[i].end > peer_ranges_[j].end) {
        need_push_.emplace_back(peer_ranges_[j].end, self_ranges_[i].end);
        need_pull_.emplace_back(peer_ranges_[j].start, self_ranges_[i].start);
        j++;
      } else if (self_ranges_[i].start >= peer_ranges_[j].start &&
                 self_ranges_[j].end <= peer_ranges_[j].end) {
        if (peer_ranges_[j].start != self_ranges_[i].start) {
          need_pull_.emplace_back(peer_ranges_[j].start, self_ranges_[i].start);
        }
        if (self_ranges_[i].end != peer_ranges_[j].end) {
          need_pull_.emplace_back(self_ranges_[i].end, peer_ranges_[j].end);
        }
        i++;
      } else if (self_ranges_[i].start <= peer_ranges_[j].start &&
                 self_ranges_[i].end >= peer_ranges_[j].end) {
        if (self_ranges_[i].start != peer_ranges_[j].start) {
          need_push_.emplace_back(self_ranges_[i].start, peer_ranges_[j].start);
        }
        if (peer_ranges_[j].end != self_ranges_[i].end) {
          need_push_.emplace_back(peer_ranges_[j].end, self_ranges_[i].end);
        }
        j++;
      }
    }
  }

  while (i < self_ranges_.size()) {
    need_push_.push_back(self_ranges_[i++]);
  }
  while (j < peer_ranges_.size()) {
    need_pull_.push_back(peer_ranges_[j++]);
  }

  LOG(INFO) << "need push ranges:";
  for (const auto &r : need_push_) {
    LOG(INFO) << r.ToString();
  }
  LOG(INFO) << "need pull rnages:";
  for (const auto &r : need_pull_) {
    LOG(INFO) << r.ToString();
  }
}

void GossipTask::PushData() {
  if (need_push_.empty()) {
    return;
  }
  brpc::Channel channel;
  brpc::ChannelOptions opt;
  int32_t ret = 0;
  if ((ret = channel.Init(peer_.c_str(), &opt)) != 0) {
    LOG(ERROR) << "init channel failed:" << ret;
    SetTaskStatus(INIT_RPC_CHANNEL_FAILED);
    SetTaskState(TASK_FIN);
    return;
  }
  brpc::Controller cntl;
  GossipData req;
  EmptyMessage rsp;
  GossipService_Stub stub(&channel);
  for (const auto &r : need_push_) {
    auto *d = req.add_ranges();
    d->set_start(r.start);
    d->set_end(r.end);
  }
  // TODO(zhangxingrui): async
  stub.PushData(&cntl, &req, &rsp, nullptr); // sync call
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "query data range failed, code:" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    SetTaskStatus(PROCESS_RPC_FAILED);
    SetTaskState(TASK_FIN);
    return;
  }
}

void GossipTask::PullData() {
  brpc::Channel channel;
  brpc::ChannelOptions opt;
  int32_t ret = 0;
  if ((ret = channel.Init(peer_.c_str(), &opt)) != 0) {
    LOG(ERROR) << "init channel failed:" << ret;
    SetTaskStatus(INIT_RPC_CHANNEL_FAILED);
    SetTaskState(TASK_FIN);
    return;
  }
  brpc::Controller cntl;
  EmptyMessage req;
  GossipData rsp;
  GossipService_Stub stub(&channel);
  // TODO(zhangxingrui): async
  stub.PullData(&cntl, &req, &rsp, nullptr); // sync call
  if (cntl.ErrorCode() != 0) {
    LOG(ERROR) << "query data range failed, code:" << cntl.ErrorCode()
               << ", msg:" << cntl.ErrorText();
    SetTaskStatus(PROCESS_RPC_FAILED);
    SetTaskState(TASK_FIN);
    return;
  }
  // write to local storage
  for (uint32_t i = 0; i < rsp.ranges_size(); ++i) {
    storage_->Write(Range(rsp.ranges(i).start(), rsp.ranges(i).end()));
  }
}
