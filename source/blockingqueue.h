#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <stdexcept>

template<typename T>
class BlockingQueue {
public:
    void push(T item) {
        {
            std::lock_guard lock(mu_);
            q_.push(std::move(item));
        }
        cv_.notify_one();
    }

    /// Blocks until an item is available or done() has been called.
    T pop() {
        std::unique_lock lock(mu_);
        cv_.wait(lock, [this]{ return !q_.empty() || done_.load(); });
        if (q_.empty()) throw std::runtime_error("BlockingQueue: done");
        T item = std::move(q_.front());
        q_.pop();
        return item;
    }

    /// Signal that no more items will be pushed; unblocks waiting pop() calls.
    void done() {
        done_ = true;
        cv_.notify_all();
    }

private:
    std::queue<T>            q_{};
    std::mutex               mu_{};
    std::condition_variable  cv_{};
    std::atomic<bool>        done_{false};
};
