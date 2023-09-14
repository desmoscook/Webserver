#include "Poller.h"
#include "EventLoop.h"

Poller::Poller(EventLoop* loop) : ownerLoop(loop) {
}

Poller::~Poller() {
}

void Poller::poll(std::vector<sp_Channel>& channel_list) {
    int event_count = epoll_wait(epoll_fd, &*events.begin(), MAX_EVENTS_SIZE, EPOLLWAIT_TIME);
    if (event_count < 0) {
        LOG("FATAL") << "epoll_wait error";
    }
    for (int i = 0; i < event_count; ++i) {
        int fd = events[i].data.fd;
        sp_Channel temp = channel_map[fd];
        temp->set_revents(events[i].events);
        channel_list.emplace_back(std::move(temp)); // 使用move不会改变shard_ptr的引用计数
    }
}

void Poller::addChannel(const sp_Channel &channel) {
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
        LOG("FATAL") << "epoll_ctl_add error";
    }
    channel_map[fd] = channel;
}

void Poller::modChannel(const sp_Channel &channel) {
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
        LOG("FATAL") << "epoll_ctl_mod error";
    }
}

void Poller::delChannel(const sp_Channel &channel) {
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0) {
        LOG("FATAL") << "epoll_ctl_del error";
    }
    channel_map.erase(fd);
}

inline void Poller::assertInLoopThread() {
    ownerLoop->assertInloopThread();
}