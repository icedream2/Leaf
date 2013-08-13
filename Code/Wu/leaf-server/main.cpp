#include "Leaf_server.h"
#include <iostream>

int main()
try {
    start_server();
    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
}
