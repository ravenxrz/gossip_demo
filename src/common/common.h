#pragma once

#include "error.h"

#include <cstdint>
#include <iostream>
#include <string>

#include "butil/string_splitter.h"

using node_id_t = uint32_t;
using ip_t = std::string;

struct Range {
  uint32_t start{0}; // inclusive
  uint32_t end{0};   // exclusive
  Range() = default;

  Range(uint32_t s, uint32_t e) {
    start = s;
    end = e;
  }

  // format start,end
  friend std::istream &operator>>(std::istream &is, Range &r) {
    std::string val;
    is >> val;
    sscanf(val.c_str(), "%d,%d", &r.start, &r.end);
    return is;
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
