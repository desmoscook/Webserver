#include "Server.h"
#include "../log/Logging.h"

Server::Server(Config config) 
    : port_(config.port_), 
      loop_(new EventLoop()) { }

Server::~Server() {
    if (listen_channel_) {
        loop_->delChannel(listen_channel_);
    }
}

void Server::start() {
    // 创建服务器 socket
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        LOG_FATAL("create listen socket error: %s", strerror(errno));
    }

    // 设置 socket 的地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定 socket
    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    int ret = 0;
    ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_FATAL("bind error");
    }
    ret = listen(listen_fd, 5);
    if (ret < 0) {
        LOG_FATAL("listen error");
    }
    
    // 设置为非阻塞
    int old_option = fcntl(listen_fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    if (fcntl(listen_fd, F_SETFL, new_option) == -1) {
        LOG_FATAL("set non-block error : %s", strerror(errno));
    }

    // 封装为 channel
    listen_channel_ = std::make_shared<Channel>(listen_fd);
    listen_channel_->set_events(EPOLLIN | EPOLLET | EPOLLRDHUP); // 监听 ET 模式
    listen_channel_->set_readCb([this] { handleNewConn(); });

    // 添加到 poller
    loop_->addChannel(listen_channel_);

    LOG_INFO("start server, listening on %d", port_);

    loop_->loop();
}

// 接收客户端连接
void Server::handleNewConn() {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_len = sizeof(clnt_addr);

    // ET 模式，使用 whlie 循环，防止错过信息
    while (true) {    
        int connfd = accept(listen_channel_->get_fd(), (struct sockaddr *)&clnt_addr, &clnt_len);
        if (connfd < 0) {
            if (errno != EAGAIN) {
                LOG_ERROR("accept error : errno is: %s", strerror(errno));
            }
            break;
        }
        if (HttpHandler::user_count_ >= MAX_EVENTS_SIZE) {
            LOG_ERROR("Internal server busy");
            break;
        }
        // 初始化对应的 httphandler 来处理这个客户端
        std::shared_ptr<Channel> clnt_channel(new Channel(connfd));
        clnt_channel->set_events(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
        clnt_channel->set_readCb([this, connfd] { thread_pool_.append(&clnts_[connfd]); });
        clnt_channel->set_writeCb([this, connfd] {
            clnts_[connfd].state_ = 1;
            thread_pool_.append(&clnts_[connfd]);
        });
        loop_->addChannel(clnt_channel);
        clnts_[connfd].init(loop_, clnt_channel, clnt_addr);

        LOG_INFO("client connect : %s", inet_ntoa(clnt_addr.sin_addr));
    }
}

void Server::handleRead(int sockfd) {
}