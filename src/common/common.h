#pragma once

#include <cstdint>
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
