#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include "../reactor/EventLoop.h"
#include "../reactor/Channel.h"
#include "../tool/Config.h"
#include "../http/HttpHandler.h"
#include "../tool/ThreadPool.h"

#include <memory>
#include <functional>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <vector>

/// 该类为单Reactor多线程模式中的主线程
/// 主要负责监听新连接的到来，然后将新连接分发给子线程进行业务处理

// linux 系统默认的最大的 fd 上限
const int MAX_FD = 1024;

class Server {
public:
    explicit Server(Config config);
    ~Server();

    void start();
    void handleNewConn();
    void handleRead(int sockfd);
    void handleWrite(HttpHandler* clnt);

private:
    int port_; // 监听的端口号

    std::vector<HttpHandler> clnts_{MAX_FD};
    std::shared_ptr<Channel> listen_channel_;   // 服务器socket的channel
    std::shared_ptr<EventLoop> loop_;           // 主线程事件循环
    ThreadPool<HttpHandler> thread_pool_;       // 线程池
};

#endif // WEBSERVER_SERVER_H