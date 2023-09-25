#include "Server.h"

void handleNewConn();

Server::Server(Config config) 
    : port_(config.port_), 
      loop_(new EventLoop()) {

    /* 开始监听 */
    // 创建服务器 socket
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    printf("listen_fd : %d", listen_fd);
    assert(listen_fd >= 0);

    // 设置 socket 的地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 设置 socket 的属性
    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 绑定 socket
    int ret = 0;
    ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret >= 0);
    ret = listen(listen_fd, 5);
    assert(ret >= 0);

    // 封装为 channel
    listen_channel_ = std::make_shared<Channel>(listen_fd);
    listen_channel_->set_events(EPOLLIN | EPOLLET | EPOLLRDHUP); // 监听 ET 模式
    listen_channel_->set_readCb(handleNewConn);

    // 添加到 poller
    loop_->addChannel(listen_channel_);

    loop_->loop();
}

Server::~Server() {
    if (listen_channel_) {
        loop_->delChannel(listen_channel_);
    }
}

// TODO
void handleNewConn() {
    printf("new connect\n");
}
