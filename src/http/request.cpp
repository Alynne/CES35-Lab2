//
// Created by Luis Holanda on 10/10/20.
//

#include "http/request.h"

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

static std::string_view
substringView(const std::string &buffer, size_t start, size_t len) {
    if (start > buffer.size()) return {};
    return std::string_view{buffer.c_str() + start,
                            std::min(buffer.size() - start, len)};
}

static std::optional<std::pair<method, url>>
parseRequestLine(const std::string &buffer) {
    method requestMethod;
    size_t advance;
    switch (buffer.at(0)) {
        case 'G':
            if (substringView(buffer, 1, 2) != "ET") {
                return {};
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
                return {};
            }
            break;
        case 'D':
            if (substringView(buffer, 1, 5) != "ELETE") {
                return {};
            }
            requestMethod = method::Delete;
            advance = 7;
            break;
        case 'H':
            if (substringView(buffer, 1, 3) != "EAD") {
                return {};
            }
            requestMethod = method::Head;
            advance = 5;
            break;
        case 'L':
            if (substringView(buffer, 1, 3) != "INK") {
                return {};
            }
            requestMethod = method::Link;
            advance = 5;
            break;
        case 'U':
            if (substringView(buffer, 1, 5) != "NLINK") {
                return {};
            }
            requestMethod = method::Unlink;
            advance = 7;
            break;
        default:
            return {};
    }

    auto urlEnd = buffer.find_first_of(' ', advance);
    if (urlEnd == std::string::npos) {
        return {};
    }

    auto url = url::parse(buffer.substr(advance, urlEnd));
    if (!url.has_value()) {
        return {};
    }

    return {{requestMethod, url.value()}};
}

std::optional<std::pair<request, bytes>> request::parse(int socket) {
    bytes buffer;
    buffer.reserve(4096);
    std::size_t off = 0;

    while (true) {
        auto received = recvFromSock(socket, buffer.data() + off, buffer.size() - off);
        if (received == -1) {

        } else if (received != 0) {
            auto lineEnd = buffer.find_first_of('\r');
            if (lineEnd == std::string::npos) {
                buffer.reserve(buffer.capacity() + 4096);
                off += received;
                continue;
            } else {
                auto requestLine = parseRequestLine(buffer);
                if (!requestLine.has_value()) {
                    return {};
                } else {
                    return {{
                                    {requestLine->first, requestLine->second},
                                    buffer.substr(lineEnd + 2)
                            }};
                }
            }
        }
    }
}

