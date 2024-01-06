#pragma once

#include "task.h"
#include "worker.h"

#include <functional>
#include <unordered_map>

using timer_task_id_t = uint32_t;
using TimerCB = std::function<void(timer_task_id_t)>;
using UnRegisterCB = std::function<void(timer_task_id_t)>;

// Thread-unsafe
class Timer {
public:
  // timer task runs on `worker`
  Timer(TaskWorker *worker) : worker_(worker) {}

  ~Timer();

  timer_task_id_t Reigster(uint32_t interval_ms, TimerCB cb);

  bool UnRegister(timer_task_id_t id, UnRegisterCB cb);

  timer_task_id_t GenTaskId() { return id_gen_++; }

private:
  TaskWorker *const worker_{nullptr};
  std::atomic<timer_task_id_t> id_gen_;
  std::unordered_map<timer_task_id_t, BaseTask *> tasks_;
};