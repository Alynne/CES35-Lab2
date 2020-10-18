
#ifndef CES35LAB2_SERVER_SERVER_H
#define CES35LAB2_SERVER_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <experimental/filesystem>
#include <string>
#include "http.h"

class http_connection {
public:
    /**
     * @brief Creates new HTTP connection handler to 
     *  communicate with a given socket.
     * @param connSocket The socket descriptor created for the connection.
     */
    http_connection(int connSocket, struct sockaddr_in clientAddr);
    virtual ~http_connection();
    /**
     * @brief The handler function. Serves the HTTP client connected.
     */
    void serve();
    /**
     * @brief Call operator, redirect to the serve() function.
     */
    void operator() (const std::experimental::filesystem::path& servingRoot) {
        this->servingRoot = servingRoot;
        serve();
    }
private:
    /**
     * @brief Completely receives requests from connected client.
     */
    http::request recvRequest();
    /**
     * @brief Completely sends response to connected client.
     */
    void send(http::response response);
    std::experimental::filesystem::path servingRoot;
    int connSocket; ///<! Socket descriptor for connection.
    struct sockaddr_in clientAddr; ///<! Address of client.
};

class http_server {
public:
    http_server(std::string host, unsigned short port, std::string dir);
    /**
     * @brief Starts and runs the HTTP server.
     */
    [[noreturn]] void run() noexcept;

    static constexpr int DEFAULT_MAX_CONNECTIONS = 20;
    static constexpr int DEFAULT_WORKER_WAIT_MSEC = 50;
private:
    void handleAcceptError() noexcept;

    struct sockaddr_in addr; ///<! Address to serve HTTP server
    int socket; ///<! Socket descriptor
    int maxConnections; ///<! Max number of concurrent connections.
    int workerWaitMSec; ///<! Wait time for a worker to finish in msec.
    std::experimental::filesystem::path serverRoot;
};

#endif
