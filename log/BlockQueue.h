#ifndef WEBSERVER_BLOCK_QUEUE_H
#define WEBSERVER_BLOCK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

#include <iostream>

template<typename T>
class BlockQueue {
public: 
    BlockQueue() = default;
    ~BlockQueue() = default;

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    int size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    void push(const T& t) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(t);
        cond_.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] () { return !queue_.empty(); });
        T front = queue_.front();
        queue_.pop();
        return front;    
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


#endif //WEBSERVER_BLOCK_QUEUE_H