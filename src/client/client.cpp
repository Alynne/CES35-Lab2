#include <fstream>
#include <iostream>
#include <sstream>
#include "client.h"
#include <optional>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string>
#include <iostream>

http_client::http_client(std::string host, std::uint16_t port){
    //Create TCP IP socket
    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    clientState = client_state::DISCONNECTED;
    //Resolving address
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    // hints - modo de configurar o socket para o tipo  de transporte
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    int resolve_status = 0;
    std::string portStr = std::to_string(port == 0 ? 80 : port);
    if ((resolve_status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res)) == 0) {
        if (res != NULL){
            serverAddr = *((struct sockaddr_in*) res->ai_addr);
            //Printing ip address for debugging
            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(res->ai_family, &(serverAddr.sin_addr), ipstr, sizeof(ipstr));
            std::cout << "  " << ipstr << std::endl;
            freeaddrinfo(res);
            if (connect(socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
                perror("connect");
            }
            clientState = client_state::CONNECTED;
        }
        else {
            std::cout << "Not able to resolve address."<< std::endl;
        }

    }
    recvBufferSize = DEFAULT_RECV_BUFFER_SIZE;
}
http_client::~http_client() {
  ::close(socket);
}

void
http_client::get(const http::url& url) {
    std::string emptyBody;
    get(url, emptyBody);
}

void
http_client::get(const http::url& url, const std::string& body) {
    if (clientState == client_state::DISCONNECTED) {
        throw std::runtime_error("http client is not connected to a remote server.");
    }
    http::request GETRequest(http::method::Get, url);
    GETRequest.initialize(socket);
    GETRequest.setContentLength(body.size());
    // Set request headers
    GETRequest.setHeader("Accept", "*/*");
    GETRequest.setHeader("Cache-Control", "no-cache");
    GETRequest.setHeader("Connection", "keep-alive");
    GETRequest.setHeader("User-Agent", "CES35Lab2/1.0 SimpleHTTPClient");
    // Send header
    GETRequest.sendHead();
    // Send body
    if (!body.empty()) {
        try {
            GETRequest.sendBodyPart(body);
        } catch (std::runtime_error& err) {
            std::stringstream ss(err.what());
            ss << "\nFailed to send GET request to " << url.getFullUrl();;
            throw std::runtime_error(ss.str());
        } catch (std::logic_error& err){
            std::stringstream ss(err.what());
            ss << "\nHTTP Client socket was disconnected abruptly." << std::endl;
            clientState = client_state::DISCONNECTED;
            throw std::runtime_error(ss.str());
        }
    }
}

bool 
http_client::saveAt(fs::path path) {
    if (path.empty()) {
        throw std::runtime_error("path is empty");
    }
    if (!path.has_filename()) {
        std::stringstream ss;
        ss << "path " << path.string() << " does not correspond to a file.";
        throw std::runtime_error(ss.str());
    }
    if (!fs::exists(path.parent_path())) {
        std::stringstream ss;
        ss << "path " << path.parent_path().string() << " does not exist.";
        throw std::runtime_error(ss.str());
    }
    std::ofstream downloadStream;
    downloadStream.open(path.string(), std::ios_base::out | std::ios_base::binary);
    // Receive header
    auto responseResult = http::response::parse(socket);
    if (!responseResult.has_value()) {
        throw std::runtime_error("Could not receive or parse response.");
        // TODO: Better error?
    }
    auto [response, leftovers] = responseResult.value();
    // for (auto& item : response.getHeaders()) {
    //     std::cout << item.first << ": " << item.second << std::endl;
    // }
    // Write initial body data, if any
    if (leftovers.size() > 0) {
        downloadStream.write(leftovers.c_str(), leftovers.size());
    }
    // Receive body
    size_t bytesReceived;
    http::bytes buffer;
    bool success = true;
    while (true) {
        buffer.resize(recvBufferSize);
        try {
            bytesReceived = response.recvBody(buffer);
        } catch (std::runtime_error& err) {
            std::cerr << err.what() << std::endl;
            success = false;
            break;
        } catch (std::logic_error& err) {
            std::cerr << "HTTP Client socket abruptly disconnected." << std::endl;
            success = false;
            break;
        }
        if (bytesReceived == 0) break;
        downloadStream.write(buffer.c_str(), bytesReceived);
    }
    downloadStream.close();
    return success;
}