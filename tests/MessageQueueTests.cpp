#include "Helper.hpp"
#include "MessageQueue.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cassert>

using namespace SimpleMessageQueue;
using ::testing::Return;

CREATE_MOCK_CLASS(ReqMock);

struct RspHolder {
public:
    int GetPayload() { return payload; }

    int payload;
};

class MockHandler {
public:
    MOCK_METHOD(int, HandleRequest, (int payload), ());
};

TEST(SimpleMessageQueueTest, AddRequestSync) {
    Queue<int, int, int> queue;

    MockHandler mockHandler;
    queue.RegisterHandler(42, [&](int payload) { return mockHandler.HandleRequest(payload); });

    EXPECT_CALL(mockHandler, HandleRequest(200)).WillOnce(Return(84));

    ReqMock reqMock;
    EXPECT_CALL(reqMock, GetKey()).WillRepeatedly(::testing::Return(42));
    EXPECT_CALL(reqMock, GetPayload()).WillOnce(::testing::Return(200));

    auto future = queue.Add(reqMock, ExecutionPolicy::SYNC);

    auto response = future.get(); // Wait for result
    EXPECT_EQ(response.GetPayload(), 84);
}

TEST(SimpleMessageQueueTest, AddRequestAsync) {
    Queue<int, int, int> queue;

    MockHandler mockHandler;
    queue.RegisterHandler(42, [&](int payload) { return mockHandler.HandleRequest(payload); });

    EXPECT_CALL(mockHandler, HandleRequest(200)).WillOnce(Return(84));

    ReqMock reqMock;
    EXPECT_CALL(reqMock, GetKey()).WillRepeatedly(::testing::Return(42));
    EXPECT_CALL(reqMock, GetPayload()).WillOnce(::testing::Return(200));

    auto future = queue.Add(reqMock, ExecutionPolicy::ASYNC);

    auto response = future.get(); // Wait for result
    EXPECT_EQ(response.GetPayload(), 84);
}

TEST(SimpleMessageQueueTest, MissingHandlerThrows) {
    Queue<int, int, int> queue;

    ReqMock reqMock;
    EXPECT_CALL(reqMock, GetKey()).WillRepeatedly(::testing::Return(42));

    EXPECT_ANY_THROW({ queue.Add(reqMock, ExecutionPolicy::ASYNC); });
}

TEST(SimpleMessageQueueTest, QueueProcessesRequestsInSyncMode) {
    Queue<int, int, int> queue;

    queue.RegisterHandler(1, [](int payload) { return payload * 2; });

    std::vector<std::future<Response<int>>> futures(10);
    std::vector<ReqMock>                    mocks(10);
    for (int i = 1; i <= 10; ++i) {
        EXPECT_CALL(mocks[i - 1], GetKey()).WillRepeatedly(::testing::Return(1));
        EXPECT_CALL(mocks[i - 1], GetPayload()).WillOnce(::testing::Return(i));

        futures[i - 1] = queue.Add(mocks[i - 1], ExecutionPolicy::SYNC);
    }

    for (int i = 1; i <= 10; ++i) {
        EXPECT_EQ(futures[i - 1].get().GetPayload(), i * 2);
    }
}

TEST(SimpleMessageQueueTest, MultiThreadedSafety) {
    Queue<int, int, int> queue;

    queue.RegisterHandler(1, [](int payload) { return payload * 2; });

    auto worker = [&](int start, int end) {
        for (int i = start; i <= end; ++i) {
            ReqMock reqMock;
            EXPECT_CALL(reqMock, GetKey()).WillRepeatedly(::testing::Return(1));
            EXPECT_CALL(reqMock, GetPayload()).WillOnce(::testing::Return(i));

            auto future = queue.Add(reqMock, ExecutionPolicy::SYNC);
            EXPECT_EQ(future.get().GetPayload(), i * 2);
        }
    };

    std::thread t1(worker, 1, 50);
    std::thread t2(worker, 51, 100);

    t1.join();
    t2.join();
}
