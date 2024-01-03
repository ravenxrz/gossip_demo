#include "task.h"
#include "waiter.h"
#include "worker.h"
#include "gtest/gtest.h"

class WaitTask : public BaseTask {
public:
  WaitTask(Waiter *waiter) : waiter_(waiter) {}

  void Run() override {
    waiter_->Done();
    delete this;
  }

private:
  Waiter *waiter_;
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

TEST(WorkerTest, SimpleTest) {
  constexpr int n = 1000 * 1000;
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
