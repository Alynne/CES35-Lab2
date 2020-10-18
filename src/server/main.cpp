#include <iostream>
#include "server.h"

int main(int argc, char* argv[]) {
     
    if (argc < 4) { // We expect 3 arguments
        std::cerr << "Error: Three arguments expected." << std::endl;
        return 1;
    }
    //First argument is hostname
    std::string host = argv[1];
    std::uint16_t port = std::stol(argv[2]);
    std::string path = argv[3];
    http_server myServer(host, port, path);
    return 0;
}