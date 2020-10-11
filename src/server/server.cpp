
#include <cerrno>
#include <chrono>
#include <iostream>
#include <forward_list>
#include <thread>
#include "server.h"

http_connection::http_connection(int connSocket, struct sockaddr_in clientAddr) 
:   connSocket(connSocket), clientAddr(clientAddr)
{}

void
http_connection::serve() {
    // TODO
}

void 
http_server::run() {
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
        std::thread connWorker(connection);
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