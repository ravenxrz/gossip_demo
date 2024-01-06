#include "storage.h"

#include <deque>

#include "butil/logging.h"
#include "gtest/gtest.h"

class TestRangeStorage : public testing::Test {
 public:
  void SetUp() override {
    storage_.Write({5, 7});
    storage_.Write({9, 11});
    storage_.Write({13, 15});
    storage_.Write({30, 35});

    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(5, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(30, 35));
  }

  MemRangeStorage storage_;
};

TEST_F(TestRangeStorage, insert_case1) {
  storage_.Write({0, 1});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 5);
    EXPECT_EQ(data[0], Range(0, 1));
    EXPECT_EQ(data[1], Range(5, 7));
    EXPECT_EQ(data[2], Range(9, 11));
    EXPECT_EQ(data[3], Range(13, 15));
    EXPECT_EQ(data[4], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, insert_case2) {
  storage_.Write({16, 17});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 5);
    EXPECT_EQ(data[0], Range(5, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(16, 17));
    EXPECT_EQ(data[4], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, insert_case3) {
  storage_.Write({25, 26});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 5);
    EXPECT_EQ(data[0], Range(5, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(25, 26));
    EXPECT_EQ(data[4], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, one_range_overlap_case1) {
  storage_.Write({1, 5});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(1, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, one_range_overlap_case2) {
  storage_.Write({7, 8});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(5, 8));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, one_range_overlap_case3) {
  storage_.Write({1, 6});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(1, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, one_range_overlap_case4) {
  storage_.Write({6, 8});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(5, 8));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, one_range_overlap_case5) {
  storage_.Write({32, 36});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(5, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 15));
    EXPECT_EQ(data[3], Range(30, 36));
  }
}

TEST_F(TestRangeStorage, one_range_overlap_case6) {
  storage_.Write({14, 16});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], Range(5, 7));
    EXPECT_EQ(data[1], Range(9, 11));
    EXPECT_EQ(data[2], Range(13, 16));
    EXPECT_EQ(data[3], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case1) {
  storage_.Write({5, 9});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], Range(5, 11));
    EXPECT_EQ(data[1], Range(13, 15));
    EXPECT_EQ(data[2], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case2) {
  storage_.Write({5, 11});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], Range(5, 11));
    EXPECT_EQ(data[1], Range(13, 15));
    EXPECT_EQ(data[2], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case3) {
  storage_.Write({7, 9});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], Range(5, 11));
    EXPECT_EQ(data[1], Range(13, 15));
    EXPECT_EQ(data[2], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case4) {
  storage_.Write({7, 11});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], Range(5, 11));
    EXPECT_EQ(data[1], Range(13, 15));
    EXPECT_EQ(data[2], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case5) {
  storage_.Write({6, 10});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], Range(5, 11));
    EXPECT_EQ(data[1], Range(13, 15));
    EXPECT_EQ(data[2], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case6) {
  storage_.Write({4, 12});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 3);
    EXPECT_EQ(data[0], Range(4, 12));
    EXPECT_EQ(data[1], Range(13, 15));
    EXPECT_EQ(data[2], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case7) {
  storage_.Write({5, 13});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], Range(5, 15));
    EXPECT_EQ(data[1], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case8) {
  storage_.Write({5, 15});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], Range(5, 15));
    EXPECT_EQ(data[1], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case9) {
  storage_.Write({7, 13});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], Range(5, 15));
    EXPECT_EQ(data[1], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case10) {
  storage_.Write({7, 15});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], Range(5, 15));
    EXPECT_EQ(data[1], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case11) {
  storage_.Write({6, 14});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], Range(5, 15));
    EXPECT_EQ(data[1], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case12) {
  storage_.Write({3, 17});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(data[0], Range(3, 17));
    EXPECT_EQ(data[1], Range(30, 35));
  }
}

TEST_F(TestRangeStorage, multi_ranges_overlap_case13) {
  storage_.Write({1, 100});
  {
    const auto& data = storage_.Read();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data[0], Range(1, 100));
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
