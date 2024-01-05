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
    SetTaskState(DIFF_RANGE);
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
  EmptyMessage req;
  QueryRangeResponse rsp;
  if (auto ret = rpc_->QueryDataRange(peer_, &req, &rsp, nullptr); ret != OK) {
    SetTaskStatus(ret);
    SetTaskState(TASK_FIN);
    return;
  }
  for (int i = 0; i < rsp.ranges_size(); ++i) {
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
  size_t i = 0;
  size_t j = 0;
  while (i < self_ranges_.size() && j < peer_ranges_.size()) {
    if (self_ranges_[i] == peer_ranges_[j]) {
      i++;
      j++;
      continue;
    }
    if (self_ranges_[i].end <= peer_ranges_[j].start) {
      need_push_.push_back(self_ranges_[i++]);
    } else if (self_ranges_[i].start >= peer_ranges_[j].end) {
      need_pull_.push_back(peer_ranges_[j++]);
    } else {
      assert(Range::IsOverlap(self_ranges_[i], peer_ranges_[j]));
      if (self_ranges_[i].start < peer_ranges_[j].start &&
          self_ranges_[i].end < peer_ranges_[j].end) {
        need_push_.emplace_back(self_ranges_[i].start, peer_ranges_[j].start);
        // cut
        peer_ranges_[j].start = self_ranges_[i].end;
        i++;
      } else if (self_ranges_[i].start > peer_ranges_[j].start &&
                 self_ranges_[i].end > peer_ranges_[j].end) {
        need_pull_.emplace_back(peer_ranges_[j].start, self_ranges_[i].start);
        // cut
        self_ranges_[i].start = peer_ranges_[j].end;
        j++;
      } else if (self_ranges_[i].start >= peer_ranges_[j].start &&
                 self_ranges_[j].end <= peer_ranges_[j].end) {
        if (peer_ranges_[j].start != self_ranges_[i].start) {
          need_pull_.emplace_back(peer_ranges_[j].start, self_ranges_[i].start);
        }
        // cut
        peer_ranges_[j].start = self_ranges_[i].end;
        i++;
      } else if (self_ranges_[i].start <= peer_ranges_[j].start &&
                 self_ranges_[i].end >= peer_ranges_[j].end) {
        if (self_ranges_[i].start != peer_ranges_[j].start) {
          need_push_.emplace_back(self_ranges_[i].start, peer_ranges_[j].start);
        }
        // cut
        self_ranges_[i].start = peer_ranges_[j].end;
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

  DLOG(INFO) << "need push ranges:";
  for (const auto &r : need_push_) {
    DLOG(INFO) << r.ToString();
  }
  DLOG(INFO) << "need pull ranges:";
  for (const auto &r : need_pull_) {
    DLOG(INFO) << r.ToString();
  }
}

void GossipTask::PushData() {
  if (need_push_.empty()) {
    return;
  }

  GossipData req;
  EmptyMessage rsp;
  for (const auto &r : need_push_) {
    auto *d = req.add_ranges();
    d->set_start(r.start);
    d->set_end(r.end);
  }
  if (auto ret = rpc_->PushData(peer_, &req, &rsp, nullptr); ret != OK) {
    SetTaskStatus(ret);
    SetTaskState(TaskFin);
    return;
  }
}

void GossipTask::PullData() {
  if (need_pull_.empty()) {
    return;
  }
  GossipData req;
  GossipData rsp;
  for (const auto &r : need_pull_) {
    auto *d = req.add_ranges();
    d->set_start(r.start);
    d->set_end(r.end);
  }
  if (auto ret = rpc_->PullData(peer_, &req, &rsp, nullptr); ret != OK) {
    SetTaskStatus(ret);
    SetTaskState(TASK_FIN);
    return;
  }
  // write to local storage
  for (int i = 0; i < rsp.ranges_size(); ++i) {
    storage_->Write(Range(rsp.ranges(i).start(), rsp.ranges(i).end()));
  }
}
