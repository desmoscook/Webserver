#include "timerQueue.h"

Timer::Timer() { 
    // TODO ：使超时时间和超时回调可以自定义
    timeout_time = time(NULL) + 20; 
    cb = std::bind(timerCallBack);
}

void timerCallBack() {

}

void TimerQueue::delTimer(Timer t) {
    
}
