#pragma once

#include "macro.h"

template <typename T>
class Singleton {
 public:
  Singleton() = default;
  static T& GetInstance() {
    static T val;
    return val;
  }

  DELETE_COPY_AND_ASSIGN(Singleton);
  DELETE_MOVE_AND_ASSIGN(Singleton);
};

#define SingletonClass(type) friend class Singleton<type>;