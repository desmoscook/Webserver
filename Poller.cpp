#include "Poller.h"
#include "EventLoop.h"
#include "log/Logging.h"

Poller::Poller(EventLoop* loop) : ownerloop_(loop) {
    // epoll_fd_ = epoll_create(5);
    // if (epoll_fd_ < 0) {
    //     LOG_FATAL("epoll_fd create error");
    // }
}

Poller::~Poller() {
}

void Poller::poll(std::vector<sp_Channel>& channel_list) {
    int event_count = epoll_wait(epoll_fd_, &*events_.begin(), MAX_EVENTS_SIZE, EPOLLWAIT_TIME);
    if (event_count < 0) {
        LOG_FATAL("epoll_wait error");
    }
    for (int i = 0; i < event_count; ++i) {
        int fd = events_[i].data.fd;
        sp_Channel temp = channel_map_[fd];
        temp->set_revents(events_[i].events);
        channel_list.emplace_back(std::move(temp)); // 使用move不会改变shard_ptr的引用计数
    }
}

void Poller::addChannel(const sp_Channel &channel) {
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        LOG_FATAL("epoll_ctl_add error");
    }
    channel_map_[fd] = channel;
}

void Poller::modChannel(const sp_Channel &channel) {
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        LOG_FATAL("epoll_ctl_mod error");
    }
}

void Poller::delChannel(const sp_Channel &channel) {
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event) < 0) {
        LOG_FATAL("epoll_ctl_del error");
    }
    channel_map_.erase(fd);
}