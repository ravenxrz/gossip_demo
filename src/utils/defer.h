#pragma once

#include <functional>
class defer {
 public:
  defer() = delete;
  defer(const std::function<void()>& func) = delete;
  defer& operator=(const std::function<void()>& func) = delete;
  defer& operator=(std::function<void()>&& func) = delete;

  defer(std::function<void()>&& func) : func_(func) {}

  ~defer() {
    if (func_ != nullptr) {
      func_();
    }
  }

  std::function<void()> func_;
};
