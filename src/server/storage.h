/**
 * This is a very simple storage implementation
 * we DO NOT persist data in the disk, just save data
 * in memory instead
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <deque>
#include <string>

struct Range {
  uint32_t start; // inclusive
  uint32_t end;   // exclusive

  Range(uint32_t s, uint32_t e) {
    start = s;
    end = e;
  }

  bool Valid() const { return start < end; }

  bool operator<(const Range &rhs) const { return start < rhs.start; }

  bool operator==(const Range &rhs) const {
    return start == rhs.start && end == rhs.end;
  }

  std::string ToString() const {
    return "[" + std::to_string(start) + ":" + std::to_string(end) + ")";
  }
};

// We only save data as range
// like [1 - 100), [2, 5)
class MemRangeStorage {
public:
  void Write(const Range &data);

  const std::deque<Range> &Read() const { return data_range_; }

private:
  static bool IsOverlap(const Range &lhs, const Range &rhs);

  std::deque<Range> data_range_;
};
