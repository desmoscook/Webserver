#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H

#include "log/Logging.h"

#include <functional>

#include <sys/epoll.h>
#include <unistd.h>

class EventLoop;

class Channel {
public:
    typedef std::function<void()> EventCallback;

    Channel() {}
    explicit Channel(int fd);
    ~Channel();
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    // 开始事件循环后，会调用poll得到就绪事件，使用该函数出合理就绪事件
    void handleEvents();

    // 设置回调函数
    void set_readCb(const EventCallback& readCb) { read_callback_ = readCb; }
    void set_writeCb(const EventCallback& writeCb) { write_callback_ = writeCb; }
    void set_errorCb(const EventCallback& errorCb) { error_callback_ = errorCb; }

    int get_fd() const { return fd_; }
    int get_events() const { return events_; }
    void set_fd(int sfd) { fd_ = sfd; }
    void set_events(int evt) { events_ = evt; }
    void set_revents(int revt) { revents_ = revt; }

private:
    void update();

    int fd_;       // Channel的fd
    int events_;   // Channel正在监听的事件
    int revents_;  // Channel就绪事件

    // 回调函数
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
};


#endif // WEBSERVER_CHANNEL_H