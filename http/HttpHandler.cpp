#include "HttpHandler.h"
#include "../reactor/EventLoop.h"
#include "Logging.h"
#include <bits/types/FILE.h>
#include <cstring>
#include <strings.h>

int HttpHandler::user_count_ = 0;

void HttpHandler::init(sp_Loop loop, sp_Channel clnt_channel, const sockaddr_in &addr) {
    loop_ = loop;
    address_ = addr;

    // 设置 close 回调
    clnt_channel_ = std::move(clnt_channel);
    clnt_channel_->set_errorCb([this] { init(); });

    // clnt 对应的超时器
    clnt_timer_ = new Timer(clnt_channel_->get_fd(), [this] { init(); });
    loop->addTimer(clnt_timer_);
    
    user_count_++;

    read_index_ = 0;
    content_length_ = 0;
    state_ = 0;
}

void HttpHandler::closeConn() {
    if (sp_Loop loop = loop_.lock()) {
        // 删除 channel
        loop->delChannel(clnt_channel_);
        clnt_channel_ = nullptr;
        // 删除 timer
        loop->delTimer(clnt_timer_);

        user_count_--;
        LOG_INFO("client close : %s", inet_ntoa(address_.sin_addr));
    }
    init();
}

// LT模式读取
bool HttpHandler::readOnce() {
    loop_.lock()->freshTimer(clnt_timer_);

    if (read_index_ >= READ_BUFFER_SIZE) {
        return false;
    }
    int read_bytes = 0;
    read_bytes = recv(clnt_channel_->get_fd(), read_buf_ + read_bytes, READ_BUFFER_SIZE, 0);
    read_index_ += read_bytes;

//    LOG_INFO("receive:\n\n%s", read_buf_);

    if (read_bytes <= 0) {
        return false;
    }
    return true;
}

// TODO: 完善并测试
bool HttpHandler::writeOnce() {
    loop_.lock()->freshTimer(clnt_timer_);

    if (bytes_to_send_ == 0) {
        clnt_channel_->set_events(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
        loop_.lock()->modChannel(clnt_channel_);
        init();
        return true;
    }
//    LOG_INFO("write:\n\n%s", write_buf_);

    int temp = 0;
    temp = write(clnt_channel_->get_fd(), write_buf_, write_index_);
    if (temp < 0) {
        if (errno == EAGAIN) {
            clnt_channel_->set_events(EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP);
            loop_.lock()->modChannel(clnt_channel_);
            return true;
        }
    }

    bytes_have_send_ += temp;
    bytes_to_send_ -= temp;
    if (bytes_to_send_ <= 0) {
        // TODO: 根据长连接
        clnt_channel_->set_events(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
        loop_.lock()->modChannel(clnt_channel_);
        // if (linger_) {
            closeConn();
            return true;
        // } else {
            // return false;
        // }
    }

    return true;
}

void HttpHandler::handleRequest() {
    HTTP_CODE read_ret = handleRead();
    // 请求不完整，需要继续读取
    if (read_ret == NO_REQUEST) {
        clnt_channel_->set_events(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
        loop_.lock()->modChannel(clnt_channel_);
        return;
    }

    bool write_ret = handleWrite(read_ret);
    if (!write_ret) {
        closeConn();
        return;
    }
    clnt_channel_->set_events(EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP);
    loop_.lock()->modChannel(clnt_channel_);
}

// 解析报文
HttpHandler::HTTP_CODE HttpHandler::handleRead() {
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = nullptr;

    while ((check_state_ == CHECK_STATE_CONTENT && line_status == LINE_OK) 
           || ((line_status = parse_line()) == LINE_OK)) {
        // 获取一行并开始解析
        text = get_line();
        cur_line_start = checked_index;

        // 根据主状态机的状态选择解析函数
        switch (check_state_) {
        case CHECK_STATE_REQUESTLINE: {
            ret = parse_request_line(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_HEADER: {
            ret = parse_request_headers(text);
            if (ret == BAD_REQUEST) {
                return BAD_REQUEST;
            } else if (ret == GET_REQUEST) {
                return parse_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT: {
            ret = parse_request_body(text);
            if (ret == GET_REQUEST) {
                return parse_request();
            }
            line_status = LINE_OPEN;
            break;
        }
        default:
            return  INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

// TODO ；完善并测试
bool HttpHandler::handleWrite(HTTP_CODE ret) {
    switch (ret) {
    case INTERNAL_ERROR: {
        add_status_line(500, "Internal Server Error");
        add_headers(0);
        break;
    }
    case BAD_REQUEST: {
        add_status_line(404, "Not Found");
        add_headers(0);
        break;
    }
    case FORBIDDEN_REQUEST: {
        add_status_line(403, "Forbidden");
        add_headers(0);
        break;
    }
    case FILE_REQUEST: {
        add_status_line(200, "OK");
        add_headers(0);
        break;
    }
    default: {
        return false;
    }
    }
    bytes_to_send_ = write_index_;
    return true;
}

void HttpHandler::init() {
    check_state_ = CHECK_STATE_REQUESTLINE;
    state_ = 0;

    method_ = GET;
    url_ = nullptr;
    linger_ = false;
    host_ = nullptr;

    cur_line_start = 0;
    checked_index = 0;
    read_index_ = 0;

    write_index_ = 0;
    bytes_to_send_ = 0;
    bytes_have_send_ = 0;

    memset(read_buf_, '\0', READ_BUFFER_SIZE);
    memset(write_buf_, '\0', WRITE_BUFFER_SIZE);
    memset(request_file_, '\0', FILENAME_MAX_SIZE);
}

// 解析请求行
HttpHandler::HTTP_CODE HttpHandler::parse_request_line(char* text) {
    // 把 method 和 url 中间的空格改为 \0，并解析 url
    url_ = strpbrk(text, " \t");
    if (!url_) {
        return BAD_REQUEST;
    }
    *url_++ = '\0';

    // 解析 method
    char* method = text;
    if (strcasecmp(method, "GET") == 0) {
        method_ = GET;
    } else if (strcasecmp(method, "POST") == 0) {
        method_ = POST;
    } else {
        return BAD_REQUEST;
    }

    // 解析 version
    url_ += strspn(url_, " \t");
    version_ = strpbrk(url_, " \t");
    if (!version_) return BAD_REQUEST;
    *version_++ = '\0';
    version_ += strspn(version_, " \t");

    // 仅支持 HTTP 1.1
    if (strcasecmp(version_, "HTTP/1.1") != 0) 
        return BAD_REQUEST;

    // url 跳过 http/https 前缀
    if (strncasecmp(url_, "http://", 7) == 0) {
        url_ += 7;
        url_ = strchr(url_, '/');
    }
    if (strncasecmp(url_, "https://", 8) == 0) {
        url_ += 8;
        url_ = strchr(url_, '/');
    }
    if (!url_ || url_[0] != '/') 
        return BAD_REQUEST;

    // 解析请求头完毕，状态机进入下一个状态
    check_state_ = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

// 解析请求头
// FIXME:视乎解析的有问题，没有修改为 CHECK_STATE_CONTANT 的语句
HttpHandler::HTTP_CODE HttpHandler::parse_request_headers(char* text) {
    if (text[0] == '\0') {
        if (content_length_ != 0) {
            check_state_ = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        } else {
            return GET_REQUEST;
        }
    } else if (strncasecmp(text, "Content-Type: application/json", 12) == 0) {
        text += 12;
        text += strspn(text, "\t");
        // 仅支持json类型的请求
        if (strcasecmp(text, "application/json") == 0) {
            content_type_ = text;
        } else {
            return NO_REQUEST;
        }
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            linger_ = true;
        }
    } else if (strncasecmp(text, "Content-length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        content_length_ = atol(text);
    } else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t");
        host_ = text;
    } else {
        LOG_INFO("unknow header: %s", text);
    }
    return NO_REQUEST;
}

HttpHandler::HTTP_CODE HttpHandler::parse_request_body(char* text) {
    if (read_index_ >= (content_length_ + cur_line_start)) {
        text[content_length_] = '\0';
        body_ = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}


// 将接收到的报文转换为 C 风格字符串，即把 \r\n 转换成 \0\0
HttpHandler::LINE_STATUS HttpHandler::parse_line() {
    char temp;
    for (; checked_index < read_index_; ++checked_index) {
        temp = read_buf_[checked_index];
        if (temp == '\r') {
            if ((checked_index + 1) == read_index_) {
                return LINE_OPEN;
            } else if (read_buf_[checked_index + 1] == '\n') {
                read_buf_[checked_index++] = '\0';
                read_buf_[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (temp == '\n') {
            if (checked_index > 1 && read_buf_[checked_index - 1] == '\r') {
                read_buf_[checked_index - 1] = '\0';
                read_buf_[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

// 根据请求的 url 和 method 解析出其请求的资源
HttpHandler::HTTP_CODE HttpHandler::parse_request() {
    strcpy(request_file_, doc_root);
    const char* p = strchr(url_, '/');

    if (*(p + 1) == '\0') { // 请求主页面
        char real_url[] = "/text.html";
        strcat(request_file_, real_url);
    } else if (strcmp(p, "/api/videotype/get") == 0) {
        LOG_INFO("getVideoType");
    } else { // 默认返回主页面
        char real_url[] = "/text.html";
        strcat(request_file_, real_url);
    }

    return FILE_REQUEST;
}

bool HttpHandler::add_response(const char* format, ...) {
    if (write_index_ >= WRITE_BUFFER_SIZE)
        return false;

    va_list arg_list;
    va_start(arg_list, format);

    int len = vsnprintf(write_buf_ + write_index_, WRITE_BUFFER_SIZE - 1 - write_index_, format, arg_list);
    if (len >= WRITE_BUFFER_SIZE - 1 - write_index_) {
        va_end(arg_list);
        return false;
    }

    write_index_ += len;
    va_end(arg_list);

    return true;
}

bool HttpHandler::add_status_line(int status, const char* title) {
    return add_response("HTTP/1.1 %d %s\r\n", status, title);
}

bool HttpHandler::add_headers(int content_length) {
    bool len = add_response("Content-Length:%d\r\n", content_length);
    bool lin = add_linger();
    bool blank = add_blank();
    return len && lin && blank;
}

bool HttpHandler::add_content(const char* content) {
    return add_response("%s", content);
}

bool HttpHandler::add_linger() {
    if (linger_ == true) {
        return add_response("Connection:%s\r\n", "keep-alive");
    } else {
        return add_response("Connection:%s\r\n", "close");
    }
}

bool HttpHandler::add_blank() {
    return add_response("\r\n");
}