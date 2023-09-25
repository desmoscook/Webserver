#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include "EventLoop.h"
#include "Channel.h"
#include "Config.h"

#include <memory>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

/// 该类为单Reactor多线程模式中的主线程
/// 主要负责监听新连接的到来，然后将新连接分发给子线程进行业务处理
class Server {
public:
    Server(Config config);
    ~Server();

private:
    int port_;
    std::shared_ptr<Channel> listen_channel_; // 服务器socket的channel
    std::unique_ptr<EventLoop> loop_;     // 主线程事件循环
};




#endif // WEBSERVER_SERVER_H