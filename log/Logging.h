#ifndef WEBSERVER_LOGGING_H
#define WEBSERVER_LOGGING_H

#include <string>
#include <mutex>
#include <thread>
#include <functional>
#include <chrono>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "BlockQueue.h"

const int LOG_BUFFER_SIZE = 1024;

#include <iostream>
#include <pthread.h>

enum {
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Log
{
public:
    //C++11以后,使用局部变量懒汉不用加锁
    static Log* get_instance() {
        static Log instance;
        return &instance;
    }

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static void* flush_log_thread(void *args) {
        Log::get_instance()->async_write_log();
        return nullptr;
    }

    void write_log(int level, int line, const char* file, const char* format, ...);
    void flush();

private:
    // 线程主函数，用于异步写日志
    void async_write_log();
    Log();
    ~Log();

    char buffer_[LOG_BUFFER_SIZE];  // 写入缓冲区
    FILE* fd_;                      // 文件的指针
    std::mutex file_mutex_;         // 文件写入的锁
    std::mutex buffer_mutex_;       // 缓冲区的访问锁
    BlockQueue<std::string> block_queue_;
};

#define LOG_INFO(format, ...) {Log::get_instance()->write_log(INFO, __LINE__, __FILE__, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) {Log::get_instance()->write_log(WARN, __LINE__, __FILE__, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) {Log::get_instance()->write_log(ERROR, __LINE__, __FILE__,format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_FATAL(format, ...) {Log::get_instance()->write_log(FATAL, __LINE__, __FILE__, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif //WEBSERVER_LOGGING_H
