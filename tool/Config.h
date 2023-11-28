#ifndef WEBSERVER_CONFIG_H
#define WEBSERVER_CONFIG_H

class Config {
public:
    Config(int port) { port_ = port; }
    ~Config() {}

    void parse_arg(int argc, char* argv[]);

    int port_;
};



#endif // WEBSERVER_CONFIG_H