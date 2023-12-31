///
/// Poller类，对epoll进行封装
/// 一个Poller类仅属于一个EventLoop，无需加锁
///
#ifndef WEBSERVER_POOLER_H
#define WEBSERVER_POOLER_H

#include "../log/Logging.h"
#include "Channel.h"

#include <vector>
#include <unordered_map>
#include <memory>

#include <sys/epoll.h>

class EventLoop;

const int EPOLLWAIT_TIME = 5000;
const int MAX_EVENTS_SIZE = 10000;

// 对epoll进行封装
class Poller {
public:
    typedef std::shared_ptr<Channel> sp_Channel;

    Poller();
    ~Poller() = default;

    // 轮询事件
    void poll(std::vector<sp_Channel>& channel_list);
    void addChannel(const sp_Channel &channel);
    void modChannel(const sp_Channel &channel);
    void delChannel(const sp_Channel &channel);

private:
    int epoll_fd_;                                    // epoll的fd
    epoll_event events_[MAX_EVENTS_SIZE];             // 存放epoll返回的events
    std::unordered_map<int, sp_Channel> channel_map_; // 存放fd和channel的映射
};


#endif // WEBSERVER_POOLER_H
