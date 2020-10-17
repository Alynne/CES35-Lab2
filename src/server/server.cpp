
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <iostream>
#include <forward_list>
#include <sstream>
#include <thread>
#include "server.h"
#include "http.h"

http_connection::http_connection(int connSocket, struct sockaddr_in clientAddr) 
:   connSocket(connSocket), 
    clientAddr(clientAddr), 
    servingRoot()
{}

http_connection::~http_connection() {
    close(connSocket);
}

void
http_connection::serve() {
    // Call recv to receive request
    try {
        auto request = recvRequest();
        http::response response(http::status::Ok); // TODO
        http::bytes data;
        //
        // Check path, respond 400 or 404 if there are any errors.
        //
        auto resourcePathStr = request.getUrl().getPath();
        if (resourcePathStr.empty()) {
            // Respond 400, no path
            response.setStatusCode(http::status::BadRequest);
            data = "Empty path";
            response.setContentLength(data.size());
        } else if (resourcePathStr[0] != '/') {
            // Respond 400, wrong format
            response.setStatusCode(http::status::BadRequest);
            data = "Bad path";
            response.setContentLength(data.size());
        } else {
            auto resourcePath = servingRoot / std::filesystem::path(resourcePathStr.substr(1));
            if (!std::filesystem::exists(resourcePath)) {
                // Respond 404, no such path
                // TODO
            }
            if (std::filesystem::is_directory(resourcePath)) {
                // Look for an "index.html" inside the directory
                resourcePath = resourcePath / std::filesystem::path("index.html");
                if (!std::filesystem::exists(resourcePath)) {
                    // Respond 404, no such index.html
                    // TODO
                }
            }
        }
        response.sendHead();
        if (response.getStatusCode().getCode() != http::status::Ok) {
            response.sendBodyPart(data);
        } else {
            // TODO
        }
        
    } catch (std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << "Aborting connection..." << std::endl;
    }
}

http::request
http_connection::recvRequest() {
    auto requestParseResult = http::request::parse(connSocket);
    if (!requestParseResult.has_value()) {
        // Some problem occured when receiving the request.
        std::stringstream ss;
        ss  << "Failed to receive and parse incoming request from " 
            << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port;
        throw std::runtime_error(ss.str());
    }
    http::request recvRequest = requestParseResult.value().first;
    http::bytes body = requestParseResult.value().second;
    //
    // Call recv to receive remaining body.
    //
    const size_t BODY_PART_SIZE = 4096;
    http::bytes remainingBody(BODY_PART_SIZE, '\0');
    size_t totalBytesReceived = 0;
    while (true) {
        size_t bytesReceived;
        try {
            bytesReceived = recvRequest.recvBody(remainingBody);
        } catch (std::logic_error& err) {
            std::stringstream ss(err.what());
            ss << std::endl << "HTTP Client socket abruptly disconnected.";
            throw std::runtime_error(ss.str());
        }
        if (bytesReceived == 0) break;
        totalBytesReceived += bytesReceived;
        if (totalBytesReceived == remainingBody.length()) {
            remainingBody.resize(remainingBody.length() + BODY_PART_SIZE);
        }
    }
    body.append(remainingBody);
    //
    // We just ignore this body stuff k k k k k k
    // Not useful for this activity
    //
    return {recvRequest};
}

void 
http_connection::send(http::response response) {

}

void 
http_server::run() noexcept {
    int openedConnections = 0;
    std::forward_list<std::thread> workers;
    while (true) {
        // Look for finished workers
        do {
            workers.remove_if(
                [&openedConnections](std::thread& t) {
                    if (t.joinable()) {
                        t.join();
                        openedConnections--;
                        return true;
                    }
                    return false;
            });
            if (openedConnections >  maxConnections) {
                // Couldn't remove any finished worker. Sleep a bit.
                std::this_thread::sleep_for(std::chrono::milliseconds(workerWaitMSec));
            }
        } while (openedConnections >  maxConnections);
        // We're on budget and may accept a new connection
        int sockfd;
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSockfd == -1) {
            handleAcceptError();
            continue;
        }
        // Spawn worker to handle connection
        http_connection connection(clientSockfd, clientAddr);
        std::thread connWorker(connection, serverRoot);
        openedConnections++;
        workers.emplace_front(std::move(connWorker));
    }
    exit(0);
}

void 
http_server::handleAcceptError() noexcept {
    // Error handling
    switch (errno)
    {
    case EAGAIN:
        std::cerr << "http_server::run(): EAGAIN, accept(2) would block." << std::endl;
        std::cerr << "Wrong socket option? Terminating..." << std::endl;
        exit(1);
    case EHOSTDOWN:
    case EHOSTUNREACH:
    case ENETDOWN:
    case ENETUNREACH:
    case ENOPROTOOPT:
    case EPROTO:
        // This errors are not generated by the accept(2) call and should have been
        // already pending. Just ignore and retry.
        break;
    case EBADF:
        std::cerr << "http_server::run(): EBADF, socket is not opened." << std::endl;
        std::cerr << "Socket initialization failed? Terminating..." << std::endl;
        exit(1);
    case ECONNABORTED:
        std::cerr << "http_server::run(): connection aborted before accepting." << std::endl;
        break;
    case EFAULT:
        std::cerr << "http_server::run(): EFAULT, corrupted addr variable." << std::endl;
        std::cerr << "Passing invalid address variable? Terminating..." << std::endl;
        exit(1);
    case EINTR:
        std::cerr << "http_server::run(): EINTR, accept(2) call interrupted." << std::endl;
        break;
    case EINVAL:
        std::cerr << "http_server::run(): EINVAL, socket not listening for connections." << std::endl;
        std::cerr << "Missing a listen(2) call at initialization? Terminating..." << std::endl;
        exit(1);
    case EOPNOTSUPP:
        std::cerr << "http_server::run(): EOPNOTSUPP, socket is not of type SOCK_STREAM." << std::endl;
        std::cerr << "Wrong socket type? Terminating..." << std::endl;
        exit(1);
    case EMFILE:
        std::cerr << "http_server::run(): ENFILE, Reached per-process limit of opened file descriptors." << std::endl;
        exit(2);
    case ENFILE:
        std::cerr << "http_server::run(): ENFILE, Reached per-process limit of opened file descriptors." << std::endl;
        exit(2);
    case ENOBUFS:
    case ENOMEM:
        std::cerr << "http_server::run(): ENOBUFS/ENOMEM, Not enough memory for new connections." << std::endl;
        break;
    case ENOTSOCK:
        std::cerr << "http_server::run(): ENOTSOCK, invalid socket descriptor." << std::endl;
        std::cerr << "Initialized invalid socket? Terminating..." << std::endl;
        exit(1);
    case EPERM:
        std::cerr << "http_server::run(): EPERM, connection blocked by firewall." << std::endl;
        break;
    default:
        std::cerr << "http_server::run(): Unknown error." << std::endl;
        exit(-1);
    }
}