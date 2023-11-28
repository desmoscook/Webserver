#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H

#include "../reactor/Channel.h"
#include "../timer/timerQueue.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#include <memory>
#include <functional>

class HttpHandler {
public:
    typedef std::shared_ptr<EventLoop> sp_Loop;
    typedef std::shared_ptr<Channel> sp_Channel;

    static int user_count_;  // 当前客户端的数量
    const char* doc_root = "/home/wsl/code/project/Webserver/root";
    static const int READ_BUFFER_SIZE = 1024;
    static const int WRITE_BUFFER_SIZE = 1024;
    static const int FILENAME_MAX_SIZE = 200;

    enum METHOD {
        GET,
        POST
    };
    enum HTTP_CODE {
        NO_REQUEST,              // 请求不完整，需要继续读取
        GET_REQUEST,             // 获得了完整的请求
        NO_RESOURCE,             // 请求的资源不存在  
        BAD_REQUEST,             // 请求语法有误
        FORBIDDEN_REQUEST,       // 请求资源没有权限访问
        FILE_REQUEST,            // 请求资源可以访问
        INTERNAL_ERROR           // 服务器内部错误
    };
    // 主状态机
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE, // 解析请求行
        CHECK_STATE_HEADER,      // 解析请求头
        CHECK_STATE_CONTENT      // 解析请求体
    };
    // 从状态机
    enum LINE_STATUS {
        LINE_OK,                 // 完整读取一行
        LINE_BAD,                // 报文语法有误
        LINE_OPEN                // 读取的行不完整
    };

public:
    HttpHandler() = default;
    ~HttpHandler() = default;

    void init(sp_Loop loop, sp_Channel clnt_channel, const sockaddr_in& addr);
    void closeConn();
    bool readOnce();
    bool writeOnce();
    void handleRequest();

    HTTP_CODE handleRead();
    bool handleWrite(HTTP_CODE ret);

    Timer* getTimer() { return clnt_timer_; }

private:
    void init();
    // 主状态机解析函数
    HTTP_CODE parse_request_line(char* text);    // 解析请求行
    HTTP_CODE parse_request_headers(char* text); // 解析请求头
    HTTP_CODE parse_request_body(char* text);    // 解析请求体

    char* get_line() { return read_buf_ + cur_line_start; }
    LINE_STATUS parse_line();

    HTTP_CODE parse_request();
    bool add_response(const char* format, ...);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content(const char* content);

    bool add_linger();
    bool add_blank();
    bool add_content_type();

public:
    int state_;

// private:
    std::weak_ptr<EventLoop> loop_; 

    sp_Channel clnt_channel_; // 客户端的channel
    sockaddr_in address_;     // 客户端的地址
    Timer* clnt_timer_;       // 对应的 timer

    char read_buf_[READ_BUFFER_SIZE];      // 读缓冲区
    char write_buf_[WRITE_BUFFER_SIZE];    // 写缓冲区
    char request_file_[FILENAME_MAX_SIZE]; // 客户端请求的目标文件的完整路径

    // read
    long read_index_;         // 最后一个字节的下一个位置
    long checked_index;       // 读取到的位置
    CHECK_STATE check_state_;
    long cur_line_start;

    // write
    int write_index_;
    int bytes_to_send_;
    int bytes_have_send_;

    // 报文信息
    METHOD method_;
    char* url_;
    char* version_;
    long content_length_;
    char* content_type_;
    char* host_;
    bool linger_;
    char* body_;
};

#endif //WEBSERVER_HTTPCONNECTION_H