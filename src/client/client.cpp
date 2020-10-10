#include <fstream>
#include <iostream>
#include <sstream>
#include "client.h"

std::optional<http::response>
http_client::get(const http::url& url) noexcept {
    std::string emptyBody;
    return get(url, emptyBody);
}

std::optional<http::response>
http_client::get(const http::url& url, const std::string& body) noexcept {
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
            std::cerr << "Failed to send GET request to " << url.getFullUrl() << std::endl
                    << err.what() << std::endl;
            return std::nullopt;
        }
    }
    // TODO: return http::response(...)
    return std::nullopt;
}

void 
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
    while (true) {
        try {
            bytesReceived = response.recvBody(buffer);
        } catch (std::runtime_error& err) {
            std::cerr << err.what() << std::endl;
            break;
        }
        if (bytesReceived == 0) break;
        downloadStream.write(buffer.c_str(), bytesReceived);
    }
    downloadStream.close();
}