#include <iostream>
#include "server.h"

int main(int argc, char* argv[]) {
     
    if (argc < 4) { // We expect 3 arguments
        std::cerr << "Error: Three arguments expected." << std::endl;
        return 1;
    }
    //First argument is hostname
    std::string host = argv[1];
    long port;
    try {
        port = std::stol(argv[2]);
    } catch (std::out_of_range& err) {
        std::cerr << "ERROR: Port " << argv[2] << "out of signed 4 byte range." << std::endl;
        exit(1);
    } catch (std::invalid_argument& err) {
        std::cerr << "ERROR: Port " << argv[2] << "is not a valid 2 byte signed integer." << std::endl;
        exit(1);
    }
    if (port <= 0) {
        std::cerr << "ERROR: Port should be positive. Specified port = " << port << std::endl;
        exit(1);
    } else if (port > std::numeric_limits<std::uint16_t>::max()) {
        std::cerr << "ERROR: Specified port " << port << " is out of bounds (2 bytes)" << std::endl;
        exit(1);
    }
    std::string path = argv[3];
    http_server myServer(host, port, path);
    myServer.run();
    return 0;
}