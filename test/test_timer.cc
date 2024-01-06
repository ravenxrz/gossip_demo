#include "task.h"
#include "time_utils.h"
#include "timer.h"
#include "waiter.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

TEST(TimerTest, test) {
  TaskWorker worker;
  worker.Start();

  Waiter waiter;
  Timer timer(&worker);
  auto pre_time = CurrentTimeInMs();
  auto delay_cb = [&pre_time, &waiter](timer_task_id_t id) {
    LOG(INFO) << "delta time:" << CurrentTimeInMs() - pre_time;
    waiter.Done();
  };
  timer.Reigster(10, delay_cb);
  waiter.Wait();
  // next round
  pre_time = CurrentTimeInMs();
  waiter.Reset();
  waiter.Wait();
}