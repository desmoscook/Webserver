#include "Channel.h"

Channel::Channel(int fd) : fd_(fd), events_(0), revents_(0) {
}

Channel::~Channel() {
    if (fd_ >= 0) { 
        close(fd_); 
    }
}

void Channel::handleEvents() {
    // 触发挂起事件，并没有触发可读事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        return;
    } 
    // 触发错误事件
    if (revents_ & EPOLLERR) {
        if (error_callback_) { error_callback_(); }
        return;
    }
    // 触发可读事件 | 高优先级可读 | 客户端关闭连接
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_callback_) { read_callback_(); }
    }
    // 触发可写事件
    if (revents_ & EPOLLOUT) {
        if (write_callback_) { write_callback_(); }
    }
}

void Channel::update() {
//    loop->updateChannel(this);
}

