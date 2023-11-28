#include <iostream>

#include "webserver/Server.h"
#include "log/Logging.h"

// 压测时，更改最大的 fd 上限
int main(int argc, char* argv[]) {
    Config config(8991);
    Server server(config);
    server.start();

    return 0;
}