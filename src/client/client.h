#ifndef CES35LAB2_CLIENT_CLIENT_H
#define CES35LAB2_CLIENT_CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <filesystem>
#include "http.h"

enum class client_state {
    DISCONNECTED = 0,
    CONNECTED = 1
};

/**
 * @class http_client
 * @brief Simple HTTP Client that sends HTTP requests and saves responses at 
 *      specified paths.
 */
class http_client {
public:
    /**
     * @brief Creates an http_client with a new TCP socket and connects
     *  to a remote server.
     * @param serverAddr A string with the address of the server (ip/hostname + port).
     */
    http_client(std::string serverAddr);
    /**
     * @brief Default destructor. Closes the TCP socket.
     */
    virtual ~http_client();
    /**
     * @brief Sends a crafted http request to the server the client is connected
     * @return A response object to collect the response to the request sent.
     */
    http::response send(const http::request& request) const ;
    /**
     * @brief Stores an http response content into a file specified by path
     */
    void saveAt(http::response& result, std::filesystem::path path) const ;
    /**
     * @brief Get http client conenction state.
     * @return The state of the client's HTTP connection.
    */
    client_state getState() const {
        return clientState;
    }
private:
    int socket; ///<! Client socket descriptor
    struct sockaddr_in serverAddr; ///<! Server address the client is connected to.
    client_state clientState;///<! Client connection status.
};

#endif
