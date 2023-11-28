cmake_minimum_required(VERSION 3.25)
project(Webserver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(Webserver
    main.cpp
    webserver/Server.cpp
    reactor/EventLoop.cpp
    reactor/Channel.cpp
    reactor/Poller.cpp
    timer/timerQueue.cpp
    log/Logging.cpp
    http/HttpHandler.cpp
)

target_include_directories(Webserver
    PRIVATE
    reactor
    timer  
    log  
    http  
    tool
)

# find_package(GTest REQUIRED)
# target_link_libraries(Webserver gtest gtest_main pthread)
target_link_libraries(Webserver pthread)