#include "task.h"
#include "time_utils.h"
#include "waiter.h"
#include "worker.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

DECLARE_uint32(task_min_delay_slice_ms);
DECLARE_uint32(task_max_delay_ms);

template <typename W> class WaitTask : public BaseTask {
public:
  WaitTask(W *waiter) : waiter_(waiter) {}

  void Run() override {
    waiter_->Done();
    delete this;
  }

private:
  W *waiter_;
};

class CountTask : public BaseTask {
public:
  CountTask(WaiterGroup *waiter, uint64_t *count)
      : waiter_(waiter), count_(count) {}

  void Run() override {
    ++(*count_);
    waiter_->Done();
    delete this;
  }

private:
  WaiterGroup *waiter_;
  uint64_t *count_;
};

class DelayTask : public BaseTask {
public:
  DelayTask(uint64_t *val, WaiterGroup *wg) : val_(val), wg_(wg) {}

  void Run() override {
    *val_ = CurrentTimeInUs();
    wg_->Done();
    delete this;
  }

private:
  uint64_t *val_;
  WaiterGroup *wg_;
};

TEST(WorkerTest, WaiterTest) {
  Waiter waiter;
  TaskWorker worker;
  worker.Start();

  worker.PushTask(new WaitTask(&waiter));
  waiter.Wait();

  waiter.Reset();
  worker.PushTask(new WaitTask(&waiter));
  waiter.Wait();
}

TEST(WorkerTest, NormalTask) {
  constexpr int n = 1000 * 10;
  WaiterGroup wg(n);
  TaskWorker worker;
  worker.Start();
  uint64_t count = 0;
  for (uint32_t i = 0; i < n; ++i) {
    worker.PushTask(new CountTask(&wg, &count));
  }
  wg.Wait();
  EXPECT_EQ(count, n);

  count = 0;
  wg.Reset(n);
  for (uint32_t i = 0; i < n; ++i) {
    worker.PushTask(new CountTask(&wg, &count));
  }
  wg.Wait();
  EXPECT_EQ(count, n);
}

TEST(WorkerTest, TaskInterleave) {
  TaskWorker worker;
  worker.Start();
  constexpr int n = 100000;
  struct Delay {
    uint64_t before_time{0}, after_time{0}, delay_time{0};

    std::string ToString() const {
      return std::to_string(before_time) + "," + std::to_string(after_time) +
             "," + std::to_string(delay_time);
    }
  };
  uint32_t max_delay_time = std::min(100U, FLAGS_task_max_delay_ms);
  std::vector<Delay> delay(n);
  WaiterGroup wg(n);
  for (int i = 0; i < n; ++i) {
    if (rand() % 2 == 0) {
      worker.PushTask(new WaitTask(&wg));
    } else {
      delay[i].before_time = CurrentTimeInUs();
      delay[i].delay_time =
          std::max(rand() % max_delay_time, FLAGS_task_min_delay_slice_ms) *
          1000;
      worker.PushDelayTask(new DelayTask(&delay[i].after_time, &wg),
                           delay[i].delay_time / 1000);
    }
  }
  wg.Wait();
  for (int i = 0; i < n; ++i) {
    VLOG(google::INFO) << "before:" << delay[i].before_time
                       << " delay:" << delay[i].delay_time << " after off:"
                       << delay[i].after_time - delay[i].before_time;
    EXPECT_TRUE(delay[i].before_time + delay[i].delay_time <=
                delay[i].after_time);
  }
}