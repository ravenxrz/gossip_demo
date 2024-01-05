#include "glog/logging.h"
#include "time_utils.h"
#include "gtest/gtest.h"

TEST(TimeTest, SimpleTest) {
  LOG(INFO) << CurrentTimeInUs();
  LOG(INFO) << CurrentTimeInMs();
}
