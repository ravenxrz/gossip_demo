#pragma one

#include "common.h"
#include "task.h"

class GossipTask : public BaseTask {
public:
  GossipTask(addr_t peer) : peer_(std::move(peer)) {
    SetTaskType(GossipTaskType);
    SetTaskState(QUERY_DATA_RANGE);
  }

  void Run() override;

private:
  
  void QueryRange();

  enum TaskState {
    QUERY_DATA_RANGE = 0,
    DIFF_RANGE = 1,
    PULL_DATA = 2,
    PUSH_DATA = 3
  };

private:
  addr_t peer_;
};
