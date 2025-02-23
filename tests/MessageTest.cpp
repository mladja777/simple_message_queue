#include "Message.hpp"
#include "Helper.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

CREATE_MOCK_CLASS(MockA)
CREATE_MOCK_CLASS(MockB)

TEST(MessageTest, ValueCheck) {
    using namespace SimpleMessageQueue;

    MockA mockA;
    EXPECT_CALL(mockA, GetKey()).WillOnce(::testing::Return(42));
    EXPECT_CALL(mockA, GetPayload()).WillOnce(::testing::Return(99));

    ASSERT_EQ(mockA.GetKey(), 42);
    ASSERT_EQ(mockA.GetPayload(), 99);
}

TEST(MessageTest, MultipleGetKeyCalls) {
    using namespace SimpleMessageQueue;

    MockA mockA;

    EXPECT_CALL(mockA, GetKey())
        .WillOnce(::testing::Return(42))
        .WillOnce(::testing::Return(100))
        .WillOnce(::testing::Return(200));

    ASSERT_EQ(mockA.GetKey(), 42);
    ASSERT_EQ(mockA.GetKey(), 100);
    ASSERT_EQ(mockA.GetKey(), 200);
}

TEST(MessageTest, MultipleGetPayloadCalls) {
    using namespace SimpleMessageQueue;

    MockA mockA;

    EXPECT_CALL(mockA, GetPayload())
        .WillOnce(::testing::Return(99))
        .WillOnce(::testing::Return(101))
        .WillOnce(::testing::Return(202));

    ASSERT_EQ(mockA.GetPayload(), 99);
    ASSERT_EQ(mockA.GetPayload(), 101);
    ASSERT_EQ(mockA.GetPayload(), 202);
}

TEST(MessageTest, StressTestMockClass) {
    using namespace SimpleMessageQueue;

    constexpr int numMoves = 1000;
    MockA         mockA;

    for (int i = 0; i < numMoves; ++i) {
        MockA tempMockA = std::move(mockA);
        mockA           = std::move(tempMockA);
    }

    EXPECT_CALL(mockA, GetKey()).WillOnce(::testing::Return(42));
    ASSERT_EQ(mockA.GetKey(), 42);
}

TEST(MessageTest, StressTestRequestResponse) {
    using namespace SimpleMessageQueue;

    constexpr int numCalls = 1000;
    MockA         mockB;

    EXPECT_CALL(mockB, GetKey()).WillRepeatedly(::testing::Return(42));
    EXPECT_CALL(mockB, GetPayload()).WillRepeatedly(::testing::Return(99));

    for (int i = 0; i < numCalls; ++i) {
        Request<int, int> req(mockB);
        ASSERT_EQ(req.GetKey(), 42);
        ASSERT_EQ(Response<int>(mockB).GetPayload(), 99);
    }
}
