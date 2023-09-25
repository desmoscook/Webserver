cmake_minimum_required(VERSION 3.25)
project(Webserver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(Webserver
    main.cpp
    Server.h
    Server.cpp
    EventLoop.cpp
    EventLoop.h
    Channel.cpp
    Channel.h
    Poller.cpp
    Poller.h
    timer/timerQueue.h
    timer/timerQueue.cpp
    timer/MyHeap.h
    log/Logging.cpp
    log/Logging.h
)

# find_package(GTest REQUIRED)
# target_link_libraries(Webserver gtest gtest_main pthread)
target_link_libraries(Webserver pthread)