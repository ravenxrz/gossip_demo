

#include "worker.h"

#include <atomic>
#include <mutex>
#include <thread>

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_uint32(task_min_delay_slice_ms, 1, "min delay time");
DEFINE_uint32(task_max_delay_ms, 60 * 1000, "max delay time");

TaskWorker::TaskWorker() {}

TaskWorker::~TaskWorker() {
  // mark thread exit
  exit_.store(true, std::memory_order_relaxed);
  cd_.notify_all();
  t_->join();
  delete t_;
}

void TaskWorker::Start() {
  std::lock_guard<std::mutex> lock_guard(mu_);
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
  std::vector<BaseTask *> doing_q_;
  {
    std::lock_guard<std::mutex> guard(mu_);
    doing_q_.swap(q_);
  }
  for (auto *task : doing_q_) {
    task->Run();
  }
  // pending_task_cnt_.fetch_sub(doing_q_.size(), std::memory_order_relaxed);
}

void TaskWorker::WaitForTask() {
  // try avoid one lock
  // if (pending_task_cnt_.load(std::memory_order_relaxed) != 0) {
  //   return;
  // }
  std::unique_lock<std::mutex> lock(mu_);
  while (!exit_.load(std::memory_order_relaxed) && q_.size() == 0) {
    cd_.wait(lock);
  }
}

void TaskWorker::PushTask(BaseTask *task) {
  // pending_task_cnt_.fetch_and(1, std::memory_order_relaxed);
  std::lock_guard<std::mutex> lock_guard(mu_);
  q_.push_back(task);
  if (q_.size() == 1) {
    cd_.notify_all();
  }
}

void TaskWorker::PushDelayTask(BaseTask *task, uint32_t delay_ms) {}
