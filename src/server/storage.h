/**
 * This is a very simple storage implementation
 * we DO NOT persist data in the disk, just save data
 * in memory instead
 */

#pragma once

#include "common.h"

#include <deque>
#include <mutex>

class RangeStorage {
public:
  virtual ~RangeStorage() = default;
  virtual void Write(const Range &data) = 0;
  virtual std::deque<Range> Read() = 0;
  virtual void Clear() = 0;

protected:
  std::deque<Range> data_range_;
};

// We only save data as range
// like [1 - 100), [2, 5)
class MemRangeStorage : public RangeStorage {
public:
  void Write(const Range &data) override;
  std::deque<Range> Read() override;
  void Clear() override;

private:
  std::mutex mu_;
};
