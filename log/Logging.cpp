#include "Logging.h"

Log::Log()
{
    memset(buffer_, '\0', LOG_BUFFER_SIZE);

    time_t t = time(NULL);
    struct tm* sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    // 根据时间命名日志文件
    char log_file_name[256] = {0};
    snprintf(log_file_name, 255, "%d-%02d-%02d.log",
             my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
    if ((fd_ = fopen(log_file_name, "a")) == nullptr) {
        throw std::runtime_error("Failed to open log file");
        exit(1);
    }
    
    // 创建线程异步写入文件
    std::thread write_log_thread(std::bind(&Log::async_write_log, this));
    write_log_thread.detach();
}

Log::~Log() {
    flush();
}

void Log::write_log(int level, int line, const char* file, const char *format, ...) {
    // 获取输出日志的文件和行号
    const char* file_name = strrchr(file, '/');
    file_name++;
    int n = snprintf(buffer_, sizeof(buffer_), "%s:%d ", file_name, line);

    // 获取日志写入的时间
    time_t t = time(NULL);
    struct tm* sys_tm = localtime(&t);
    struct tm this_tm = *sys_tm;

    // 判断日志等级
    char s[16] = {0};
    switch (level) {
    case INFO:
        strcpy(s, "[INFO]: ");
        break;
    case WARN:
        strcpy(s, "[WARN]: ");
        break;
    case ERROR:
        strcpy(s, "[ERROR]: ");
        break;
    case FATAL:
        strcpy(s, "[FATAL]: ");
        break;
    default:
        strcpy(s, "[INFO]: ");
        break;
    }

    // 格式化输出内容
    {
        std::lock_guard<std::mutex> locker(mutex_);    
        // 获取可变参数列表
        va_list valist;
        va_start(valist, format);
        // 写入缓冲区
        int m = snprintf(buffer_ + n, 48, "%d-%02d-%02d %02d:%02d %s ",
                         this_tm.tm_year + 1900, this_tm.tm_mon + 1, this_tm.tm_mday,
                         this_tm.tm_hour, this_tm.tm_min, s);
        int mn = vsnprintf(buffer_ + m, LOG_BUFFER_SIZE - 1, format, valist);
        buffer_[n + m + mn] = '\n';
        buffer_[n + m + mn + 1] = '\0';

        // 写入string
        std::string log_str = buffer_;
        block_queue_.push(log_str);

        va_end(valist);

        // if (level == FATAL) {
        //     fprintf(stderr, "FATAL: %s", buffer_);
        //     exit(1);
        // }
    }
}

void Log::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    fflush(fd_);
}


void Log::async_write_log() {
    std::string single_log;
    while (true) {
        // 从阻塞队列读日志并写入
        single_log = block_queue_.pop();
        mutex_.lock();
        fputs(single_log.c_str(), fd_);
        mutex_.unlock();
    }
}