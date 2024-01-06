#include <queue>
#include <thread>
#include "mutex_lock.h"

#include "gtest/gtest.h"

TEST(MutexTest, SimpleCounter) {
  struct Adder1 {
    Adder1(int* val) : val_(val) {}
    void operator()(Mutex* mu, int count) {
      mu->Lock();
      for (int i = 0; i < count; i++) {
        (*val_)++;
      }
      mu->Unlock();
    }
    int* val_;
  };

  struct Adder2 {
    Adder2(int* val) : val_(val) {}
    void operator()(Mutex* mu, int count) {
      LockGuard guard(mu);
      for (int i = 0; i < count; i++) {
        (*val_)++;
      }
    }
    int* val_;
  };

  constexpr int tn = 10;
  std::vector<std::thread> threads;
  threads.reserve(tn);
  int add_per_thread = 1000 * 100;
  int count = 0;
  Mutex mu;
  for (int i = 0; i < tn; ++i) {
    if (i & 1) {
      threads.emplace_back(Adder1(&count), &mu, add_per_thread);
    } else {
      threads.emplace_back(Adder2(&count), &mu, add_per_thread);
    }
  }
  for (auto& t : threads) {
    t.join();
  }
  EXPECT_EQ(count, add_per_thread * tn);
}

TEST(CondVarTest, SimpleQueue) {
  Mutex mu;
  CondVar cv;
  std::queue<int> q;
  struct Producure {
    Producure(std::queue<int>* q) : q_(q) {}
    void operator()(CondVar* cv) {
      usleep(10000);
      q_->push(1);
      if (rand() & 1) {
        cv->NotifyAll();
      } else {
        cv->NotifyOne();
      }
    }

    std::queue<int>* q_;
  };

  std::thread t(Producure(&q), &cv);
  LockGuard guard(&mu);
  cv.Wait(&mu, [&q] { return !q.empty(); });
  t.join();
  EXPECT_EQ(q.size(), 1);
}
