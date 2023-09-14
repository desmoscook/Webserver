//
// Created by lenovo on 2023/9/12.
//

#ifndef WEBSERVER_LOGGING_H
#define WEBSERVER_LOGGING_H

#include <iostream>

#define LOG(level) std::cout << "[" << level << "] "

// class LOG_INFO {
// public:
//     LOG_INFO& operator<<(const char* str) {
//         printf("[INFO]  %s\n", str);
//         return *this;
//     }
// };

// class LOG_FATAL {
// public:
//     LOG_FATAL& operator<<(const char* str) {
//         printf("[FATAL]  %s\n", str);
//         return *this;
//     }
// };

#endif //WEBSERVER_LOGGING_H
