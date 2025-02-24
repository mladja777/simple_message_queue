#ifndef SMQ_MESSAGEQUEUE_HPP_
#define SMQ_MESSAGEQUEUE_HPP_

#include "Message.hpp"
#include <cassert>
#include <future>
#include <map>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <queue>

namespace SimpleMessageQueue {

enum class ExecutionPolicy { SYNC, ASYNC };

template<typename ReqKey = int, typename ReqPayload = int, typename RspPayload = int>
class Queue {
public:
    using Request  = ::SimpleMessageQueue::Request<ReqKey, ReqPayload>;
    using Response = ::SimpleMessageQueue::Response<RspPayload>;
    using Handler  = std::function<RspPayload(ReqPayload)>;
    using Item     = std::pair<Request, std::promise<SimpleMessageQueue::Response<RspPayload>>>;

    ~Queue() {
        mShouldExecuteQueued.store(false);
        mQueueThread.join();
    }

    std::future<Response> Add(Request request, ExecutionPolicy policy = ExecutionPolicy::SYNC) {
        {
            std::lock_guard<std::mutex> lock(mHandlersMutex);
            if (!mHandlers.contains(request.GetKey())) {
                throw std::invalid_argument("No available handlers to process this request.");
            }
        }

        if (ExecutionPolicy::SYNC == policy) {
            std::promise<SimpleMessageQueue::Response<RspPayload>> promise;
            std::future<SimpleMessageQueue::Response<RspPayload>>  future = promise.get_future();

            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                mQueue.push({ std::move(request), std::move(promise) });
            }

            return std::move(future);
        } else {
            return std::async(
                std::launch::async,
                [&](Request request) {
                    std::lock_guard<std::mutex> lock(mHandlersMutex);
                    assert(mHandlers.find(request.GetKey()) != mHandlers.end());
                    return SimpleMessageQueue::Response<RspPayload>(
                        mHandlers.at(request.GetKey())(request.GetPayload()));
                },
                std::move(request));
        }
    }

    void RegisterHandler(ReqKey key, Handler handler) {
        std::lock_guard<std::mutex> lock(mHandlersMutex);
        mHandlers[key] = handler;
    }

private:
    void ExecuteSync() {
        while (mShouldExecuteQueued.load()) {
            if ([&]() {
                    std::lock_guard<std::mutex> lock(mQueueMutex);
                    return mQueue.empty();
                }()) {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(100ms);
                continue;
            }

            auto [request, promise] = [&]() {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                auto                        front = std::move(mQueue.front());
                mQueue.pop();
                return std::move(front);
            }();

            promise.set_value([&]() {
                std::lock_guard<std::mutex> lock(mHandlersMutex);
                assert(mHandlers.find(request.GetKey()) != mHandlers.end());
                return SimpleMessageQueue::Response<RspPayload>(
                    mHandlers.at(request.GetKey())(request.GetPayload()));
            }());
        }
    }

    std::queue<Item>  mQueue;
    std::mutex        mQueueMutex;
    std::thread       mQueueThread { &Queue::ExecuteSync, this };
    std::atomic<bool> mShouldExecuteQueued { true };

    std::map<ReqKey, Handler> mHandlers;
    std::mutex                mHandlersMutex;
};

} // namespace SimpleMessageQueue

#endif // !SMQ_MESSAGEQUEUE_HPP_
