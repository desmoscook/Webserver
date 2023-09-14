#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H

#include "log/Logging.h"

#include <functional>

#include <sys/epoll.h>

class EventLoop;

class Channel {
public:
    typedef std::function<void()> EventCallback;

    explicit Channel(int fd);
    ~Channel();
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    // 开始事件循环后，会调用poll得到就绪事件，使用该函数出合理就绪事件
    void handleEvents();

    // 设置回调函数
    void setReadCallback(const EventCallback& readCb) { readCallback = readCb; }
    void setWriteCallback(const EventCallback& writeCb) { writeCallback = writeCb; }
    void setErrorCallback(const EventCallback& errorCb) { errorCallback = errorCb; }

    int get_fd() const { return fd; }
    int get_events() const { return events; }
    int set_fd(int sfd) { fd = sfd; }
    int set_events(int evt) { events = evt; }
    void set_revents(int revt) { revents = revt; }

private:
    void update();

    int fd;       // Channel的fd
    int events;   // Channel正在监听的事件
    int revents;  // Channel就绪事件

    // 回调函数
    EventCallback readCallback;
    EventCallback writeCallback;
    EventCallback errorCallback;
};


#endif // WEBSERVER_CHANNEL_H