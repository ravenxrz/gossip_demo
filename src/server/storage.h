/**
 * This is a very simple storage implementation
 * we DO NOT persist data in the disk, just save data
 * in memory instead
 */

#pragma once

#include "common.h"

#include <cassert>
#include <cstdint>
#include <deque>
#include <string>

class RangeStorage {
public:
  virtual ~RangeStorage() = default;
  virtual void Write(const Range &data) = 0;

  const std::deque<Range> &Read() const { return data_range_; }

protected:
  static bool IsOverlap(const Range &lhs, const Range &rhs);

  std::deque<Range> data_range_;
};

// We only save data as range
// like [1 - 100), [2, 5)
class MemRangeStorage : public RangeStorage {
public:
  void Write(const Range &data) override;
};
