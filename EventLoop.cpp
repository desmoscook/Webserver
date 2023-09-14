#include "log/Logging.h"
#include "EventLoop.h"

// 线程局部存储的变量，指向当前线程的EventLoop对象，每个线程只能有一个EventLoop对象
__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : is_looping(false),
      threadId_(std::this_thread::get_id())
{
    LOG("INFO") << "EventLoop created" << std::endl;
    if (t_loopInThisThread == nullptr) {
        t_loopInThisThread = this;
    } else {
        LOG("FATAL") << "Another Eventloop is exist" << std::endl;
        abort();
    }
}

EventLoop::~EventLoop() {
    assert(!is_looping);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!is_looping);
    assert(isInloopThread());
    is_looping = true;;
    is_stop = false;

    while (!is_stop) {
        // 1.epoll_wait阻塞，等待就绪事件
        active_channels.clear();
        poller->poll(active_channels); // 调用poller将就绪事件放入active_channels中
        // 2.调用channel函数对就绪事件进行处理
        is_event_handing = true;
        for (auto i : active_channels) {
            i->handleEvents();
        }
        is_event_handing = false;
        // 3.执行正在等待的函数
        handlePendingFunctions();
        // 4.处理超时事件
    }

    std::cout << "EventLoop " << this << " stop looping" << std::endl;
    is_looping = false;
}

void EventLoop::stopLoop() {
    is_stop = true;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

void EventLoop::handlePendingFunctions() {

}





