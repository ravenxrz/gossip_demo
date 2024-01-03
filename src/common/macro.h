#pragma once

#define DELETE_COPY_AND_ASSIGN(cls)                                            \
  cls(const cls &) = delete;                                                   \
  cls &operator=(const cls &) = delete

#define DELETE_MOVE_AND_ASSIGN(cls)                                            \
  cls(cls &&) = delete;                                                        \
  cls &operator=(cls &&) = delete
