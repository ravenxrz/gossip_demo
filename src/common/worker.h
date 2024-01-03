#pragma once

#include "task.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include "gflags/gflags_declare.h"
#include "gflags/gflags.h"

// Simple worker, supports two ops:
// 1. PushTask, worker will run task immediately if it's not busy
// 2. PushDelayTask(delay ms) worker will run it after `delay` ms
class TaskWorker {
public:
  TaskWorker();

  ~TaskWorker();

  void Start();

  void PushTask(BaseTask *task);

  void PushDelayTask(BaseTask *task, uint32_t delay_ms);

private:
  void ThreadMain();

  void Handle();

  void WaitForTask();

  std::thread *t_{nullptr};
  std::atomic<bool> exit_{false};
  std::mutex mu_;
  std::condition_variable cd_;

  std::atomic<uint64_t> pending_task_cnt_{0};
  std::vector<BaseTask *> q_;

  std::atomic<uint64_t> pending_delay_task_cnt_{0};
  std::vector<std::set<BaseTask *>> *delay_q_;
  uint64_t handle_time_;   // current delay slice
  uint64_t handle_cursor_; // current delay cursor
};
