cmake_minimum_required(VERSION 3.16)
project(Webserver)

set(CMAKE_CXX_STANDARD 17)

add_executable(Webserver
#        main.cpp
        EventLoop.cpp
        EventLoop.h
        Channel.cpp
        Channel.h
        Poller.cpp
        Poller.h
        log/Logging.cpp
        log/Logging.h
        log/Singleton.h
        S00/test.cpp
)

find_package(GTest REQUIRED)
target_link_libraries(Webserver pthread GTest::GTest GTest::Main)