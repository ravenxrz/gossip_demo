#pragma once

#include "common.h"
#include "igossip_rpc.h"
#include "storage.h"
#include "task.h"

#include <vector>

class GossipTask : public BaseTask {
public:
  GossipTask(addr_t peer, RangeStorage *storage, IGossipRpc *rpc)
      : peer_(std::move(peer)), storage_(storage), rpc_(rpc) {
    SetTaskType(GossipTaskType);
    SetTaskState(QUERY_DATA_RANGE);
  }

  void Run() override;

private:
  void QueryRange();

  void ReadLocalRange();

  void Diff();

  void PushData();

  void PullData();

  enum TaskState {
    QUERY_DATA_RANGE = 0,
    READ_LOCAL_RANGE,
    DIFF_RANGE,
    PULL_DATA,
    PUSH_DATA,
    TASK_FIN
  };

private:
  addr_t peer_;
  RangeStorage *storage_;
  std::vector<Range> self_ranges_;
  std::vector<Range> peer_ranges_;
  std::vector<Range> need_push_;
  std::vector<Range> need_pull_;

  IGossipRpc *rpc_;
};
