#include <fstream>
#include <iostream>
#include <sstream>
#include "client.h"

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
http_client::saveAt(http::response& response, std::filesystem::path path) {
    if (path.empty()) {
        throw std::runtime_error("path is empty");
    }
    if (!path.has_filename()) {
        std::stringstream ss;
        ss << "path " << path.string() << " does not correspond to a file.";
        throw std::runtime_error(ss.str());
    }
    if (!std::filesystem::exists(path.parent_path())) {
        std::stringstream ss;
        ss << "path " << path.parent_path().string() << " does not exist.";
        throw std::runtime_error(ss.str());
    }
    std::ofstream downloadStream;
    downloadStream.open(path.string(), std::ios_base::out | std::ios_base::binary);
    // Receive header
    // TODO
    // Receive body
    size_t bytesReceived;
    http::bytes buffer(recvBufferSize, 0);
    bool success = true;
    while (true) {
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