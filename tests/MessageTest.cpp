#include "Message.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define CREATE_MOCK_CLASS(ClassName)                                                               \
    class ClassName {                                                                              \
    public:                                                                                        \
        ClassName()                 = default;                                                     \
        ClassName(const ClassName&) = default;                                                     \
        ClassName(ClassName&& other) noexcept                                                      \
            : mockGetKey(std::move(other.mockGetKey))                                              \
            , mockGetPayload(std::move(other.mockGetPayload)) {}                                   \
        ClassName& operator=(const ClassName& other) = default;                                    \
        ClassName& operator=(ClassName&& other) noexcept {                                         \
            if (this != &other) {                                                                  \
                mockGetKey     = std::move(other.mockGetKey);                                      \
                mockGetPayload = std::move(other.mockGetPayload);                                  \
            }                                                                                      \
            return *this;                                                                          \
        }                                                                                          \
                                                                                                   \
        MOCK_METHOD(int, GetKey, (), (const));                                                     \
        MOCK_METHOD(int, GetPayload, (), (const));                                                 \
                                                                                                   \
    private:                                                                                       \
        std::unique_ptr<testing::internal::FunctionMocker<int()>> mockGetKey;                      \
        std::unique_ptr<testing::internal::FunctionMocker<int()>> mockGetPayload;                  \
    };

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

TEST(MessageTest, StressTestMockMethods) {
    using namespace SimpleMessageQueue;

    constexpr int numCalls = 1000;
    MockA         mockB;

    EXPECT_CALL(mockB, GetKey()).WillRepeatedly(::testing::Return(42));
    EXPECT_CALL(mockB, GetPayload()).WillRepeatedly(::testing::Return(99));

    for (int i = 0; i < numCalls; ++i) {
        ASSERT_EQ(mockB.GetKey(), 42);
        ASSERT_EQ(mockB.GetPayload(), 99);
    }
}
