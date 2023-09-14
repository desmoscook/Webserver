#include "Channel.h"

Channel::Channel(int fd) : fd(fd), events(0), revents(0) {
}

Channel::~Channel() {
}

void Channel::handleEvents() {
    // 触发挂起事件，并没有触发可读事件
    if ((revents & EPOLLHUP) && !(revents & EPOLLIN)) {
        return;
    } 
    // 触发错误事件
    if (revents & EPOLLERR) {
        if (errorCallback) { errorCallback(); }
        return;
    }
    // 触发可读事件 | 高优先级可读 | 客户端关闭连接
    if (revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback) { readCallback(); }
    }
    // 触发可写事件
    if (revents & EPOLLOUT) {
        if (writeCallback) { writeCallback(); }
    }
}

void Channel::update() {
//    loop->updateChannel(this);
}

