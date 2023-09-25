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

class Timer {
public:
    typedef std::function<void()> TimerCallback;
    friend TimerQueue;

    Timer();
    ~Timer() {}

    bool operator<(const Timer& timer) const {
        return timeout_time < timer.timeout_time;
    }

private:
    time_t timeout_time; // 超时时间
    TimerCallback cb;    // 超时回调
};


class TimerQueue {
public:
    TimerQueue() {}
    ~TimerQueue() {}

    void addTimer(Timer t) { timer_queue_.add(t); }
    void delTimer(Timer t);
    void freshTimer(Timer t);

private:
    MyHeap<Timer> timer_queue_; // 定时器堆，是小根堆
    std::unordered_map<int, Timer> timer_map_; // 定时器哈希表，用于快速查找定时器
};

// TODO : 超时回调函数实现
void timerCallBack();

#endif // WEBSERVER_TIMERQUEUE_H