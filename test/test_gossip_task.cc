
#include "common.h"
#include "data.pb.h"
#include "gossip.pb.h"
#include "gossip_task.h"
#include "igossip_rpc.h"
#include "storage.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;

class MockGossipRpc : public IGossipRpc {
public:
  MockGossipRpc();

  MOCK_METHOD(int32_t, QueryDataRange,
              (const std::string &addr, EmptyMessage *req,
               QueryRangeResponse *rsp, google::protobuf::Closure *done),
              (override));

  MOCK_METHOD(int32_t, PushData,
              (const std::string &addr, GossipData *req, EmptyMessage *rsp,
               google::protobuf::Closure *done),
              (override));

  MOCK_METHOD(int32_t, PullData,
              (const std::string &addr, GossipData *req, GossipData *rsp,
               google::protobuf::Closure *done),
              (override));
};

MockGossipRpc::MockGossipRpc() {
  EXPECT_CALL(*this, PushData).WillRepeatedly(Return(OK));
}

class GossipTest : public testing::Test {
public:
  void SetUp() override {
    storage_.Write(::Range(10, 20));
    storage_.Write(::Range(30, 40));
    storage_.Write(::Range(50, 60));
  }

  MemRangeStorage storage_;
};

TEST_F(GossipTest, NoNeedGossip) {
  MockGossipRpc rpc;
  auto query_data_range_handle =
      [](const std::string &addr, EmptyMessage *req, QueryRangeResponse *rsp,
         google::protobuf::Closure *done) -> int32_t {
    std::vector<::Range> ranges = {{10, 20}, {30, 40}, {50, 60}};
    for (const auto &r : ranges) {
      auto *d = rsp->add_ranges();
      d->set_start(r.start);
      d->set_end(r.end);
    }
    return OK;
  };
  EXPECT_CALL(rpc, QueryDataRange).WillOnce(query_data_range_handle);
  EXPECT_CALL(rpc, PullData).Times(0);
  EXPECT_CALL(rpc, PushData).Times(0);

  GossipTask *task = new GossipTask("", &storage_, &rpc);
  task->Run();

  const auto &r = storage_.Read();
  EXPECT_EQ(r.size(), 3);
  EXPECT_EQ(r[0], ::Range(10, 20));
  EXPECT_EQ(r[1], ::Range(30, 40));
  EXPECT_EQ(r[2], ::Range(50, 60));
}

TEST_F(GossipTest, OnlyPush) {
  MockGossipRpc rpc;
  EXPECT_CALL(rpc, QueryDataRange).WillOnce(Return(OK));
  EXPECT_CALL(rpc, PushData)
      .WillOnce([](const std::string &addr, GossipData *req, EmptyMessage *rsp,
                   google::protobuf::Closure *done) {
        EXPECT_EQ(req->ranges_size(), 3);
        EXPECT_EQ(req->ranges(0).start(), 10);
        EXPECT_EQ(req->ranges(0).end(), 20);
        EXPECT_EQ(req->ranges(1).start(), 30);
        EXPECT_EQ(req->ranges(1).end(), 40);
        EXPECT_EQ(req->ranges(2).start(), 50);
        EXPECT_EQ(req->ranges(2).end(), 60);
        return OK;
      });
  EXPECT_CALL(rpc, PullData).Times(0);

  GossipTask *task = new GossipTask("", &storage_, &rpc);
  task->Run();
}

TEST_F(GossipTest, OnlyPull) {
  MockGossipRpc rpc;
  auto query_handle = [](const std::string &addr, EmptyMessage *req,
                         QueryRangeResponse *rsp,
                         google::protobuf::Closure *done) {
    std::vector<::Range> ranges = {{10, 20}, {30, 40}, {50, 60}, {70, 80}};
    for (const auto &r : ranges) {
      auto *d = rsp->add_ranges();
      d->set_start(r.start);
      d->set_end(r.end);
    }
    return OK;
  };
  EXPECT_CALL(rpc, QueryDataRange).WillOnce(query_handle);
  EXPECT_CALL(rpc, PullData)
      .WillOnce([](const std::string &addr, GossipData *req, GossipData *rsp,
                   google::protobuf::Closure *done) {
        for (int i = 0; i < req->ranges_size(); ++i) {
          auto *d = rsp->add_ranges();
          d->set_start(req->ranges(i).start());
          d->set_end(req->ranges(i).end());
        }
        EXPECT_EQ(rsp->ranges().size(), 1);
        EXPECT_EQ(rsp->ranges(0).start(), 70);
        EXPECT_EQ(rsp->ranges(0).end(), 80);
        return OK;
      });
  EXPECT_CALL(rpc, PushData).Times(0);
  GossipTask *task = new GossipTask("", &storage_, &rpc);
  task->Run();
}

TEST_F(GossipTest, PushAndPull) {
  storage_.Write(::Range(70, 80));

  MockGossipRpc rpc;
  auto query_handle = [](const std::string &addr, EmptyMessage *req,
                         QueryRangeResponse *rsp,
                         google::protobuf::Closure *done) {
    std::vector<::Range> ranges = {{5, 15}, {35, 45}, {48, 65}, {75, 78}};
    for (const auto &r : ranges) {
      auto *d = rsp->add_ranges();
      d->set_start(r.start);
      d->set_end(r.end);
    }
    return OK;
  };
  EXPECT_CALL(rpc, QueryDataRange).WillOnce(query_handle);
  auto push_handle = [](const std::string &addr, GossipData *req,
                        EmptyMessage *rsp, google::protobuf::Closure *done) {
    std::vector<::Range> expected_r = {{15, 20}, {30, 35}, {70, 75}, {78, 80}};
    EXPECT_EQ(req->ranges_size(), expected_r.size());
    for (int i = 0; i < req->ranges_size(); ++i) {
      EXPECT_EQ(req->ranges(i).start(), expected_r[i].start);
      EXPECT_EQ(req->ranges(i).end(), expected_r[i].end);
    }
    return OK;
  };
  EXPECT_CALL(rpc, PushData).WillOnce(push_handle);
  auto pull_handle = [](const std::string &addr, GossipData *req,
                        GossipData *rsp, google::protobuf::Closure *done) {
    std::vector<::Range> expected_r = {{5, 10}, {40, 45}, {48, 50}, {60, 65}};
    EXPECT_EQ(req->ranges_size(), expected_r.size());
    for (int i = 0; i < req->ranges_size(); ++i) {
      EXPECT_EQ(req->ranges(i).start(), expected_r[i].start);
      EXPECT_EQ(req->ranges(i).end(), expected_r[i].end);
      auto *d = rsp->add_ranges();
      d->set_start(expected_r[i].start);
      d->set_end(expected_r[i].end);
    }
    return OK;
  };
  EXPECT_CALL(rpc, PullData).WillOnce(pull_handle);
  GossipTask *task = new GossipTask("", &storage_, &rpc);
  task->Run();

  std::vector<::Range> expects = {{5, 20}, {30, 45}, {48, 65}, {70, 80}};
  const auto &actual = storage_.Read();
  EXPECT_EQ(expects.size(), actual.size());
  for (uint32_t i = 0; i < expects.size(); ++i) {
    EXPECT_EQ(expects[i], actual[i]);
  }
}