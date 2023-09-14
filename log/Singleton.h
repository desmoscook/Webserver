//
// Created by lenovo on 2023/9/12.
//

#ifndef WEBSERVER_SINGLETON_H
#define WEBSERVER_SINGLETON_H


class Singleton {
public:
    static Singleton* getInstance() { return instance_; }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
private:
    Singleton() = default;

    static Singleton* instance_;
};


#endif //WEBSERVER_SINGLETON_H
