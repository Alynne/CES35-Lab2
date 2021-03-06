//
// Created by Luis Holanda on 10/10/20.
//

#include "http/request.h"
#include <cerrno>
#include <chrono>
#include <iostream>
#include <thread>

using namespace http;

void request::writeStartLine(bytes &buffer) const noexcept {
    switch (mMethod) {
        case method::Get:
            buffer += "GET ";
            break;
        case method::Post:
            buffer += "POST ";
            break;
        case method::Put:
            buffer += "PUT ";
            break;
        case method::Delete:
            buffer += "DELETE ";
            break;
        case method::Head:
            buffer += "HEAD ";
            break;
        case method::Link:
            buffer += "LINK ";
        case method::Unlink:
            buffer += "UNLINK ";
    }

    buffer += getUrl().getFullUrl();
    buffer += " HTTP/1.0\r\n";
}

static std::optional<std::pair<method, url>>
parseRequestLine(const std::string &buffer) {
    method requestMethod;
    size_t advance;
    switch (buffer.at(0)) {
        case 'G':
            if (substringView(buffer, 1, 2) != "ET") {
                return std::nullopt;
            }
            requestMethod = method::Get;
            advance = 4;
            break;
        case 'P':
            if (substringView(buffer, 1, 2) == "UT") {
                requestMethod = method::Put;
                advance = 4;
            } else if (substringView(buffer, 1, 3) == "OST") {
                requestMethod = method::Post;
                advance = 5;
            } else {
                return std::nullopt;
            }
            break;
        case 'D':
            if (substringView(buffer, 1, 5) != "ELETE") {
                return std::nullopt;
            }
            requestMethod = method::Delete;
            advance = 7;
            break;
        case 'H':
            if (substringView(buffer, 1, 3) != "EAD") {
                return std::nullopt;
            }
            requestMethod = method::Head;
            advance = 5;
            break;
        case 'L':
            if (substringView(buffer, 1, 3) != "INK") {
                return std::nullopt;
            }
            requestMethod = method::Link;
            advance = 5;
            break;
        case 'U':
            if (substringView(buffer, 1, 5) != "NLINK") {
                return std::nullopt;
            }
            requestMethod = method::Unlink;
            advance = 7;
            break;
        default:
            return std::nullopt;
    }

    auto urlEnd = buffer.find_first_of(' ', advance);
    if (urlEnd == std::string::npos) {
        return std::nullopt;
    }

    auto url = url::parse(buffer.substr(advance, urlEnd-advance));
    if (!url.has_value()) {
        return std::nullopt;
    }

    return {{requestMethod, url.value()}};
}

std::optional<std::pair<request, bytes>> request::parse(int socket) {
    bytes buffer;
    buffer.resize(4096);
    std::size_t off = 0;
    std::optional<request> parsedRequest;
    // Parse request line.
    while (true) {
        auto received = recvFromSock(socket, buffer.data() + off, buffer.size() - off);
        //std::cout << "bytes received: " << received << std::endl;
        if (received == -1) {
            throw std::runtime_error("error while receiving request header");
        } else if (received != 0) {
            off += received;
            //std::cout << "content received: " << buffer << "END";
            auto lineEnd = buffer.find_first_of('\r');
            if (lineEnd == std::string::npos) {
                buffer.resize(buffer.size() + 4096);
                continue;
            } else {
                auto requestLine = parseRequestLine(buffer);
                if (!requestLine.has_value()) {
                    return std::nullopt;
                } else {
                    parsedRequest.emplace(requestLine->first, requestLine->second);
                    buffer.resize(off);
                    off = lineEnd + 2;
                    break;
                }
            }
        } else {
            // No more data to receive but found no "\r\n"
            if (off == 0) throw std::runtime_error("connection finished but did not receive anything.");
            throw std::runtime_error("connection finished while receiving first request header line.");
        }
    }
    parsedRequest->initialize(socket);
    auto body = parsedRequest->parseHeaders(buffer, off);
    if (body.has_value()) {
        return {{parsedRequest.value(), body.value()}};
    } else {
        return std::nullopt;
    }
}

