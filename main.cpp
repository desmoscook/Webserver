#include <iostream>

#include "Server.h"
#include "log/Logging.h"

int main(int argc, char* argv[]) {
    Config config(8991);
    Server server(config);

    return 0;
}