#include "gossip_task.h"

void GossipTask::Run() {
  switch (GetTaskState()) {
  case QUERY_DATA_RANGE:
    break;
  case DIFF_RANGE:
    break;
  case PULL_DATA:
    break;
  case PUSH_DATA:
    break;
  default:
    break;
  }
}
