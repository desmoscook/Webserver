#include <cstdio>
#include <iostream>
#include <cassert>
#include <ostream>
#include <unistd.h>
#include "../timer/MyHeap.h"
#include "../timer/timerQueue.h"

typedef std::function<void()> TimerCallback;

void onTimer1() {
    std::cout << "onTimer1 called" << std::endl;
}

void onTimer2() {
    std::cout << "onTimer2 called" << std::endl;
}

int main() {
    // auto* t1 = new Timer(1, onTimer1); // 0
    // sleep(3);
    // auto t2 = new Timer(2, onTimer2); // 3
    // sleep(3);
    // auto t3 = new Timer(3, onTimer1); // 6

    // MyHeap<Timer*, cmp> hh;
    // hh.add(t2);
    // hh.add(t3);
    // hh.add(t1);

    // for (auto i : hh.heap_) {
    //     std::cout << "timeout :" << i->timeout_time_
    //               << " fd : " << i->fd_ << std::endl;
    // }

    // sleep(3);
    // t1->fresh(); // 9
    // hh.adjust();

    // printf("\n");

    // for (auto i : hh.heap_) {
    //     std::cout << "timeout :" << i->timeout_time_
    //               << " fd : " << i->fd_ << std::endl;
    // }
}