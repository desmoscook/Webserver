#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include "Poller.h"

#include <thread>
#include <memory>

#include <assert.h>
#include <unistd.h>

#include <iostream>
#include <functional>

class Channel;

class EventLoop {
public:
    typedef std::function<void> Function;

    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 停止事件循环，不会立即生效，即不会停止当前循环，会停止下一轮的循环
    void stopLoop();

    void addChannel(const std::shared_ptr<Channel>& channel) { poller->addChannel(channel); }
    void modChannel(const std::shared_ptr<Channel>& channel) { poller->modChannel(channel); }
    void delChannel(const std::shared_ptr<Channel>& channel) { poller->delChannel(channel); }

    bool isInloopThread() const { return threadId_ == std::this_thread::get_id(); }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    typedef std::shared_ptr<Channel> sp_Channel;

    void handlePendingFunctions(); // 指定等待的函数(Subloop注册EpollAdd连接socket，以及绑定事件的函数)

    std::unique_ptr<Poller> poller; // 轮询器(IO多路复用分发器)
    std::vector<sp_Channel> active_channels; // 有事件(激活的)的channel

    int event_fd; // 用于异步唤醒的eventfd
    sp_Channel wakeup_channel; // 用于异步唤醒的channel

    std::vector<Function> pending_functions; // 等待处理的函数

    bool is_looping; // 是否正在事件循环
    bool is_stop; // 是否停止事件循环
    bool is_event_handing; // 是否正在处理事件
    bool is_calling_pending_functions; // 是否正在调用等待处理的函数

    const std::thread::id threadId_;
};


#endif //WEBSERVER_EVENTLOOP_H
