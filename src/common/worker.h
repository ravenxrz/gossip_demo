#pragma once

#include "mutex_lock.h"
#include "task.h"

#include <atomic>
#include <queue>
#include <set>
#include <thread>

#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"

// Simple worker, supports two ops:
// 1. PushTask, worker will run task immediately if it's not busy
// 2. PushDelayTask(delay ms) worker will run it after `delay` ms
class TaskWorker {
 public:
  TaskWorker();

  ~TaskWorker();

  void Start();

  void PushTask(BaseTask* task);

  // Notice: it's not accurate
  void PushDelayTask(BaseTask* task, uint32_t delay_ms);

 private:
  void ThreadMain();

  void Handle();

  void WaitForTask();

  std::thread* t_{nullptr};
  std::atomic<bool> exit_{false};
  Mutex mu_;
  CondVar cv_;

  std::vector<BaseTask*> q_;
  std::set<BaseTask*>* delay_q_{nullptr};
  int64_t base_time_us_{0};
  int32_t cursor_{0};
  static int64_t min_time_slice_us_;
  static uint32_t time_slice_cnt_;
};
