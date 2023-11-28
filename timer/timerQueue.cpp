#include "timerQueue.h"

#include <bits/types/time_t.h>
#include <utility>

Timer::Timer(int fd, TimerCallback cb) : 
    fd_(fd),
    timeout_time_(time(nullptr) + 30),
    cb_(std::move(cb)) {}

void Timer::fresh() {
    timeout_time_ = time(nullptr) + 30;
}

bool TimerQueue::isTimeOut() {
    if (timer_queue_.empty()) return false;
    time_t t = time(nullptr);
    if (t > timer_queue_.top()->timeout_time_) {
        return true;
    }
    return false;
}

void TimerQueue::handleTimeOut() {
    time_t t = time(nullptr);
    while (!timer_queue_.empty()) {
        Timer* front = timer_queue_.top();
        if (t < front->timeout_time_) break;
        front->cb_();
    }
}

void TimerQueue::freshTimer(Timer* t) {
    t->fresh();
    timer_queue_.adjust();
}
