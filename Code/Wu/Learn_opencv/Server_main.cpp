#include "Leaf_server.h"
#include <stdexcept>
#include <iostream>

int main(int argc, char* argv[])
try {
    using Server = Leaf::Server::Tcp_server;
    Server server;
    server.run();

    return 0;
}
catch (std::exception& e){
    std::cerr << e.what() << std::endl;
    return 1;
}

//-------------------------------------------------------
// End of File
//-------------------------------------------------------
