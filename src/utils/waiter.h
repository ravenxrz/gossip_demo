#include <cassert>

#include "mutex_lock.h"

class WaiterGroup {
public:
  WaiterGroup(uint32_t wait_num) : wait_num_(wait_num) {}

  void Wait() {
    LockGuard guard(&mu_);
    // std::unique_lock<std::mutex> lck(mu_);
    cv_.Wait(&mu_, [this] { return wait_num_ == 0; });
  }

  void Done() {
    LockGuard guard(&mu_);
    if (wait_num_-- == 1) {
      cv_.NotifyAll();
    }
  }

  void Reset(uint32_t num) {
    LockGuard guard(&mu_);
    assert(wait_num_ == 0);
    wait_num_ = num;
  }

private:
  uint32_t wait_num_{0};
  Mutex mu_;
  CondVar cv_;
};

class Waiter {
public:
  Waiter() : wg(1) {}

  void Wait() { wg.Wait(); }

  void Done() { wg.Done(); }

  void Reset() { wg.Reset(1); }

private:
  WaiterGroup wg;
};