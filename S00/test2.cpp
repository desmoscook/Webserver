//
// Created by lenovo on 2023/9/12.
//

#include <iostream>
#include "../EventLoop.h"

EventLoop* g_loop;

void threadFunc() {
    g_loop->loop();
}

int main() {
    EventLoop loop;
    g_loop = &loop;

    std::thread thread(threadFunc);
    thread.join();
}