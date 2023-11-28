#ifndef WEBSERVER_TIMERQUEUE_H
#define WEBSERVER_TIMERQUEUE_H

#include "MyHeap.h"


#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
#include <unordered_map>

#include <time.h>
#include <assert.h>

class TimerQueue;
class cmp;

class Timer {
public:
    typedef std::function<void()> TimerCallback;
    friend TimerQueue;
    friend cmp;

    Timer(int fd, TimerCallback cb);
    ~Timer() = default;

    void fresh();

    bool operator==(const Timer& timer) const {
        return fd_ == timer.fd_;
    }

private:
    int fd_;              // 对应的clnt
    time_t timeout_time_; // 超时时间
    TimerCallback cb_;    // 超时回调
};

class cmp {
public:
    bool operator()(Timer* t1, Timer* t2) {
        return t1->timeout_time_ < t2->timeout_time_;
    }
};


class TimerQueue {
public:
    TimerQueue() = default;
    ~TimerQueue() = default;

    bool isTimeOut();
    void handleTimeOut();

    void addTimer(Timer* t) { timer_queue_.add(t); }
    void delTimer(Timer* t) { timer_queue_.del(t); }
    void freshTimer(Timer* t);

private:
    MyHeap<Timer*, cmp> timer_queue_; // 定时器堆，是小根堆
    // std::unordered_map<int, Timer> timer_map_; // 定时器哈希表，用于快速查找定时器
};

#endif // WEBSERVER_TIMERQUEUE_H