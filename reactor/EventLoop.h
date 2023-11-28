#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include "Poller.h"
#include "../timer/timerQueue.h"
#include "../tool/ThreadPool.h"

#include <assert.h>
#include <unistd.h>

#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <memory>

class Channel;

class EventLoop {
public:
    typedef std::shared_ptr<Channel> sp_Channel;
    // typedef std::function<void> Function;

    EventLoop();
    ~EventLoop();

    void loop();
    void stopLoop();

    void addChannel(const sp_Channel& channel) { poller_->addChannel(channel); }
    void modChannel(const sp_Channel& channel) { poller_->modChannel(channel); }
    void delChannel(const sp_Channel& channel) { poller_->delChannel(channel); }

    void addTimer(Timer* t) { timer_.addTimer(t); }
    void delTimer(Timer* t) { timer_.delTimer(t); }
    void freshTimer(Timer* t) { timer_.freshTimer(t); }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    std::unique_ptr<Poller> poller_;          // 轮询器(IO多路复用分发器)
    std::vector<sp_Channel> active_channels_; // 有事件(激活的)的channel

    int event_fd_;              // 用于异步唤醒的eventfd
    sp_Channel wakeup_channel_; // 用于异步唤醒的channel
    TimerQueue timer_;          // 超时队列
    // std::vector<Function> pending_functions; // 等待处理的函数

    bool is_looping_;                    // 是否正在事件循环
    bool is_stop_;                       // 是否停止事件循环
    bool is_event_handing_;              // 是否正在处理事件
    bool is_calling_pending_functions_;  // 是否正在调用等待处理的函数

    const std::thread::id threadId_;
};


#endif //WEBSERVER_EVENTLOOP_H
