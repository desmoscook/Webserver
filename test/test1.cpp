//
// Created by lenovo on 2023/9/12.
//

#include <iostream>
#include <thread>
#include "../EventLoop.h"

void threadFunc() {
//    printf("threadFunc() : pid = %d, tid = %d\n",
//           getpid(), std::this_thread::get_id());
    std::cout << "threadFunc() : pid = " << getpid() << ", tid = " << std::this_thread::get_id() << std::endl;

    EventLoop loop;
    loop.loop();
}

int main() {
//    printf("threadFunc() : pid = %d, tid = %d\n",
//           getpid(), std::this_thread::get_id());
    std::cout << "threadFunc() : pid = " << getpid() << ", tid = " << std::this_thread::get_id() << std::endl;

    EventLoop loop;

    std::thread thread(threadFunc);
    thread.detach();

    loop.loop();
}