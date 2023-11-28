#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include "../log/Logging.h"

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <list>
#include <vector>

template<typename T>
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool() = default;

    bool append(T* request);

private:
    void run();

private:
    int thread_num_;
    int max_requests_;
    std::vector<std::thread> threads_;           // 线程数组
    std::list<T *> work_queue_;                  // 请求队列
    std::mutex queue_mutex_;                     // 保护请求队列的互斥锁
    std::condition_variable queue_not_empty_;    // 是否有任务需要处理
};

template<typename T>
ThreadPool<T>::ThreadPool() {
    thread_num_ = 16;
    max_requests_ = 10000;
    threads_.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
        threads_.emplace_back([this] { run(); });
        threads_.back().detach();
    }
}

template<typename T>
bool ThreadPool<T>::append(T* request) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (work_queue_.size() > max_requests_) return false;
    // 添加任务
    work_queue_.push_back(request);
    queue_not_empty_.notify_one();
    return true;
}

template<typename T>
void ThreadPool<T>::run() {
    T* request = nullptr;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // 处理虚假唤醒
            while (work_queue_.empty()) {
                queue_not_empty_.wait(lock, [this] { return !work_queue_.empty(); });
            }
            request = work_queue_.front();
            work_queue_.pop_front();
        }
        if (work_queue_.empty()) continue;
        // 现在在处理任务之前已经获取到了互斥锁，确保只有一个线程处理任务
        if (!request) continue;
        // 开始处理
        if (request->state_ == 0) {
            if (request->readOnce()) {
                request->handleRequest();
            }
        } else {
            if (!request->writeOnce()) {
                LOG_ERROR("write error");
            }
        }
    }
}

#endif //WEBSERVER_THREADPOOL_H
