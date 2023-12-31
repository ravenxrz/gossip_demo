#include "storage.h"

#include <algorithm>
#include <iostream>
#include <mutex>

#include "error.h"
#include "glog/logging.h"
#include "utils/mutex_lock.h"

namespace {

bool StartCompare(const Range& lhs, const Range& rhs) {
  return lhs.start < rhs.start;
}

bool EndCompare(const Range& lhs, const Range& rhs) {
  return lhs.end < rhs.end;
}

}  // namespace

// TODO(zhangxingrui): Maybe use interval tree is much better
void MemRangeStorage::Write(const Range& data) {
  DLOG(INFO) << "receive " << data.ToString();
  LockGuard guard(&mu_);
  if (data_range_.empty()) {
    data_range_.push_back(data);
    return;
  }
  uint32_t start = data.start;
  uint32_t end = data.end;
  auto left = std::lower_bound(data_range_.begin(), data_range_.end(), data,
                               StartCompare);
  if (left == data_range_.end()) {
    if (Range::IsOverlap(*(left - 1), data)) {
      (left - 1)->end = data.end;
    } else {
      data_range_.push_back(data);
    }
    return;
  }
  if (left != data_range_.begin()) {
    auto may_left = left - 1;
    bool o1 = Range::IsOverlap(*may_left, data);
    bool o2 = Range::IsOverlap(*left, data);
    if (!o1 && !o2) {
      data_range_.insert(left, data);
      return;
    } else if (o1) {
      left = may_left;
      start = left->start;
    }  // !o1
  } else {
    if (!Range::IsOverlap(*left, data)) {
      data_range_.push_front(data);
      return;
    }
  }

  auto right = std::lower_bound(data_range_.begin(), data_range_.end(), data,
                                EndCompare);
  // erase操作,会使得迭代器失效,
  // 记住left的idx
  int pos = left - data_range_.begin();
  if (right != data_range_.end()) {
    if (Range::IsOverlap(*right, data)) {
      end = std::max(end, right->end);
      data_range_.erase(left + 1, right + 1);
    } else {
      if (left < right) {
        data_range_.erase(left + 1, right);
      }  // if left == right, do nothing
    }
  } else {
    data_range_.erase(left + 1, right);
  }
  data_range_[pos].start = start;
  data_range_[pos].end = end;
}

std::deque<Range> MemRangeStorage ::Read() {
  LockGuard lck(&mu_);
  return data_range_;
}

void MemRangeStorage::Clear() {
  LockGuard lck(&mu_);
  data_range_.clear();
}
