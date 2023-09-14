#include <iostream>

#include "log/Logging.h"

int main() {
    int a = 0;
    LOG("INFO") << "hello" << "tjs" << a << "this" << std::endl;
    LOG("FATAL") << "error";
    return 0;
}
