#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>

#include "macro.h"

class Mutex {
public:
  DELETE_COPY_AND_ASSIGN(Mutex);

  Mutex() {}

  void Lock() { lck_.lock(); }

  bool TryLock() { return lck_.try_lock(); }

  void Unlock() { lck_.unlock(); }

private:
  friend class CondVar;
  std::mutex lck_;
};

class LockGuard {
public:
  LockGuard(Mutex *mu) : mu_(mu) { mu_->Lock(); }

  ~LockGuard() { mu_->Unlock(); }

private:
  Mutex *mu_;
};

class CondVar {
public:
  CondVar() {}

  // wait until stop_waiting returns true
  template <typename Predicate>
  void Wait(Mutex *mu, Predicate stop_waiting = nullptr) {
    std::unique_lock<std::mutex> lck(mu->lck_, std::adopt_lock);
    cv_.wait(lck, stop_waiting);
    lck.release();
  }

  template <class Rep, class Period, class Predicate>
  bool WaitFor(Mutex *mu, const std::chrono::duration<Rep, Period> &rel_time,
               Predicate stop_waiting = nullptr) {
    std::unique_lock<std::mutex> lck(mu->lck_, std::adopt_lock);
    bool ret = cv_.wait_for(lck, rel_time, stop_waiting);
    lck.release();
    return ret;
  }

  void NotifyOne() { cv_.notify_one(); }

  void NotifyAll() { cv_.notify_all(); }

private:
  std::condition_variable cv_;
};