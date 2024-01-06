

#include "worker.h"
#include "mutex_lock.h"
#include "task.h"
#include "utils/time_utils.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <mutex>
#include <ratio>
#include <thread>

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_uint32(
    task_min_delay_slice_ms, 1,
    "task min delay time(inclusive), doesnt support modify at run time");
// max delay time: 60s
DEFINE_uint32(
    task_max_delay_ms, 60 * 1000 + 1,
    "task max delay time(exclusive), doesnt support modify at run time");
static bool validate_task_min_delay_slice_ms(const char *name, uint32_t val) {
  return val >= 1;
}
static bool validate_task_max_delay_ms(const char *name, uint32_t val) {
  return val >= FLAGS_task_min_delay_slice_ms;
}
DEFINE_validator(task_min_delay_slice_ms, validate_task_min_delay_slice_ms);
DEFINE_validator(task_max_delay_ms, validate_task_max_delay_ms);

int64_t TaskWorker::min_time_slice_us_;
uint32_t TaskWorker::time_slice_cnt_;

TaskWorker::TaskWorker() : base_time_us_(0), cursor_(0) {
  time_slice_cnt_ = FLAGS_task_max_delay_ms / FLAGS_task_min_delay_slice_ms;
  delay_q_ = new std::set<BaseTask *>[time_slice_cnt_];
  min_time_slice_us_ = FLAGS_task_min_delay_slice_ms * 1000;
}

TaskWorker::~TaskWorker() {
  // mark thread exit
  exit_.store(true, std::memory_order_relaxed);
  cv_.NotifyAll();
  t_->join();
  delete t_;

  // delete not running task
  for (auto *task : q_) {
    delete task;
  }
  for (uint32_t i = 0; i < time_slice_cnt_; ++i) {
    for (auto *task : delay_q_[i]) {
      delete task;
    }
  }
  delete[] delay_q_;
}

void TaskWorker::Start() {
  LockGuard guard(&mu_);
  if (t_) {
    return;
  }
  t_ = new std::thread(&TaskWorker::ThreadMain, this);
}

void TaskWorker::ThreadMain() {
  while (!exit_.load(std::memory_order_relaxed)) {
    Handle();
    WaitForTask();
  }
}

void TaskWorker::Handle() {
  std::vector<BaseTask *> doing_q;
  std::set<BaseTask *> delay_doing_q;
  {
    LockGuard guard(&mu_);
    if (!q_.empty()) {
      doing_q.swap(q_);
    }
    if (!delay_q_[cursor_ % time_slice_cnt_].empty()) {
      delay_doing_q.swap(delay_q_[cursor_ % time_slice_cnt_]);
    }
  }
  for (auto *task : doing_q) {
    task->Run();
  }
  int64_t now = CurrentTimeInUs();
  for (auto *task : delay_doing_q) {
    assert(now >= task->GetTimeTicket());
    task->Run();
  }
}

void TaskWorker::WaitForTask() {
  LockGuard guard(&mu_);
  if (!q_.empty()) {
    return;
  }
  int64_t now_us = CurrentTimeInUs();
  while (base_time_us_ != 0 && now_us >= base_time_us_) {
    if (!delay_q_[cursor_ % time_slice_cnt_].empty()) {
      return;
    }
    ++cursor_;
    base_time_us_ += min_time_slice_us_;
  }
  uint32_t wait_time = min_time_slice_us_;
  if (base_time_us_ != 0) {
    cursor_ = (cursor_ - 1) % time_slice_cnt_;
    base_time_us_ -= min_time_slice_us_;
    wait_time = (min_time_slice_us_ - (now_us - base_time_us_));
  }

  if (cv_.WaitFor(&mu_,
                  std::chrono::microseconds(wait_time), // go to next time slice
                  [this] { return !q_.empty() || exit_; })) {
    if (base_time_us_ != 0) {
      now_us = CurrentTimeInUs();
      if (auto delta = now_us - (base_time_us_ + min_time_slice_us_);
          delta < 0) {
        // usleep(-delta);
        return;
      }
      cursor_++;
      base_time_us_ += min_time_slice_us_;
    }
  }
}

void TaskWorker::PushTask(BaseTask *task) {
  assert(task->GetWorker() == nullptr || task->GetWorker() == this);
  task->SetWorker(this);
  LockGuard guard(&mu_);
  q_.push_back(task);
  if (q_.size() == 1) {
    cv_.NotifyAll();
  }
}

void TaskWorker::PushDelayTask(BaseTask *task, uint32_t delay_ms) {
  assert(delay_ms >= FLAGS_task_min_delay_slice_ms);
  assert(delay_ms < FLAGS_task_max_delay_ms);
  assert(task->GetWorker() == nullptr || task->GetWorker() == this);
  task->SetWorker(this);
  int64_t now_us = CurrentTimeInUs();
  task->SetTimeTicket(now_us + delay_ms * 1000);
  LockGuard guard(&mu_);
  if (base_time_us_ == 0) {
    base_time_us_ = now_us;
    cursor_ = 0;
  } else {
    delay_ms += (now_us - base_time_us_ + 999) / 1000;
    if (delay_ms >= FLAGS_task_max_delay_ms) {
      // worker is so busy
      LOG(WARNING) << "worker is busy, shrink delay time to "
                   << FLAGS_task_max_delay_ms - 1;
      delay_ms = FLAGS_task_max_delay_ms - 1;
    }
  }
  uint32_t delta_slice = delay_ms * 1000 / min_time_slice_us_;
  uint32_t slot = (delta_slice + cursor_) % time_slice_cnt_;
  assert(task->GetTimeTicket() <=
         base_time_us_ + (delta_slice * min_time_slice_us_));
  delay_q_[slot].insert(task);
}
