#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H

#include <sys/socket.h>
#include <arpa/inet.h>


class HttpConnection {
public:
    HttpConnection();
    ~HttpConnection();

private:
    int m_sockfd_;          // 客户端的socket
    sockaddr_in m_address_; // 客户端的地址
};



#endif //WEBSERVER_HTTPCONNECTION_H