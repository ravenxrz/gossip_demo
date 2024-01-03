#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>

class WaiterGroup {
public:
  WaiterGroup(uint32_t wait_num) : wait_num_(wait_num) {}

  void Wait() {
    std::unique_lock<std::mutex> lck(mu_);
    cd_.wait(lck, [this] { return wait_num_ == 0; });
  }

  void Done() {
    std::unique_lock<std::mutex> lck(mu_);
    if (wait_num_-- == 1) {
      cd_.notify_all();
    }
  }

  void Reset(uint32_t num) {
    std::lock_guard<std::mutex> guard(mu_);
    assert(wait_num_ == 0);
    wait_num_ = num;
  }

private:
  uint32_t wait_num_{0};
  std::mutex mu_;
  std::condition_variable cd_;
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