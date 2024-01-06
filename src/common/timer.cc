#include "timer.h"
#include "task.h"
#include "waiter.h"

#include <chrono>

#include "glog/logging.h"

namespace {

using StopCB = UnRegisterCB;

class TimerTask : public BaseTask {
public:
  TimerTask(timer_task_id_t id, uint32_t delay_ms, TimerCB timer_cb)
      : id_(id), delay_ms_(delay_ms), timer_cb_(std::move(timer_cb)) {}

  void Run() override {
    if (stoped_) {
      if (stop_cb_) {
        stop_cb_(id_);
      }
      TaskDone();
      return;
    }
    timer_cb_(id_);
    worker_->PushDelayTask(this, delay_ms_);
  }

  void Stop(StopCB cb) {
    stop_cb_ = std::move(cb);
    stoped_ = true;
  }

private:
  timer_task_id_t id_;
  uint32_t delay_ms_;
  TimerCB timer_cb_;
  StopCB stop_cb_{nullptr};
  std::atomic<bool> stoped_{false};
};
} // namespace

Timer::~Timer() {
  Waiter waiter;
  for (const auto &[id, task] : tasks_) {
    static_cast<TimerTask *>(task)->Stop(
        [&waiter](timer_task_id_t) { waiter.Done(); });
    waiter.Wait();
    waiter.Reset();
  }
}

timer_task_id_t Timer::Reigster(uint32_t interval_ms, TimerCB cb) {
  auto id = GenTaskId();
  auto *task = new TimerTask(id, interval_ms, cb);
  tasks_[id] = task;
  worker_->PushDelayTask(task, interval_ms);
  return id;
}

bool Timer::UnRegister(timer_task_id_t id, UnRegisterCB cb) {
  auto it = tasks_.find(id);
  if (it == tasks_.end()) {
    LOG(ERROR) << "no such timer task id:" << id;
    return false;
  }
  auto *task = it->second;
  tasks_.erase(it);
  static_cast<TimerTask *>(task)->Stop(cb);
  return true;
}