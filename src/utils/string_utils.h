#pragma once

#include "butil/string_splitter.h"
#include "butil/strings/string_piece.h"

inline void SplitString(const std::string &raw_str, char d,
                        std::vector<std::string> *result) {
  result->clear();
  for (butil::StringSplitter splitter(butil::StringPiece(raw_str), d); splitter;
       splitter++) {
    result->emplace_back(splitter.field(), splitter.length());
  }
}

inline void StrTrim(std::string *raw_str) {
  // remove last spaces first
  auto lpos = raw_str->find_last_not_of(' ');
  if (lpos != std::string::npos) {
    raw_str->erase(raw_str->begin() + lpos + 1, raw_str->end());
  }
  auto fpos = raw_str->find_first_not_of(' ');
  if (fpos != std::string::npos) {
    raw_str->erase(raw_str->begin(), raw_str->begin() + fpos);
  }
}

inline bool StrStartsWith(const std::string &raw_str,
                          const std::string &start) {
  return raw_str.find(start) == 0;
}
