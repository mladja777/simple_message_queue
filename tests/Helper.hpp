#ifndef SMQ_HELPER_HPP_
#define SMQ_HELPER_HPP_

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

#endif // !SMQ_HELPER_HPP_
