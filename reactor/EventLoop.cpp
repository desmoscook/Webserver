#include "../log/Logging.h"
#include "EventLoop.h"


// 线程局部存储的变量，指向当前线程的EventLoop对象，每个线程只能有一个EventLoop对象
__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : is_looping_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new Poller())
{
    LOG_INFO("EventLoop created");
    if (t_loopInThisThread == nullptr) {
        t_loopInThisThread = this;
    } else {
        LOG_FATAL("Another Eventloop is exist");
        abort();
    }
}

EventLoop::~EventLoop() {
    assert(!is_looping_);
    t_loopInThisThread = nullptr;
}

// 开启事件循环
void EventLoop::loop() {
    assert(!is_looping_);

    while (!is_stop_) {
        // 1.epoll_wait 阻塞，等待就绪事件
        active_channels_.clear();
        poller_->poll(active_channels_); // 调用 poller 将就绪事件放入 active_channels 中

        // 2.调用channel函数对就绪事件进行处理
        is_event_handing_ = true;
        for (const auto& i : active_channels_) {
            i->handleEvents();
        }
        is_event_handing_ = false;

        // 3.处理超时事件
        if (timer_.isTimeOut()) {
            timer_.handleTimeOut();
        }
    }

    LOG_INFO("EventLoop stop looping");
}

// 停止事件循环，不会立即生效(不会停止当前循环，会停止下一轮的循环)
void EventLoop::stopLoop() {
    is_stop_ = true;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}