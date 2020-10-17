//
// Created by Luis Holanda on 10/10/20.
//

#include "http/request.h"
#include <cerrno>

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

std::string_view
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

    auto url = url::parse(buffer.substr(advance, urlEnd));
    if (!url.has_value()) {
        return std::nullopt;
    }

    return {{requestMethod, url.value()}};
}

std::optional<std::pair<request, bytes>> request::parse(int socket) {
    bytes buffer;
    buffer.reserve(4096);
    std::size_t off = 0;
    std::optional<request> parsedRequest;

    // Parse request line.
    while (true) {
        auto received = recvFromSock(socket, buffer.data() + off, buffer.capacity() - off);
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
                    return std::nullopt;
                } else {
                    parsedRequest.emplace(requestLine->first, requestLine->second);
                    off = lineEnd + 2;
                    break;
                }
            }
        }
    }

    // Parse headers.
    std::size_t headerStart = off;
    while(true) {
        auto nextEnd = buffer.find_first_of('\r', headerStart);
        if (nextEnd == std::string::npos || (nextEnd <= buffer.size() && nextEnd > buffer.size() - 4)) {
            buffer.reserve(1024);
            auto received = recvFromSock(socket, buffer.data() + off, buffer.capacity() - off);
            if (received == -1) {
            } else if (received != 0) {
                off += received;
            }
        } else {
            auto headersFinished = buffer[nextEnd + 1] == '\n'
                    && buffer[nextEnd + 2] == '\r'
                    && buffer[nextEnd + 3] == '\n';
            if (headersFinished) {
                return {{parsedRequest.value(), buffer.substr(headersFinished + 4) }};
            } else {
                auto headerName = buffer.find_first_of(':', headerStart);
                if (headerName == std::string::npos) {
                    return std::nullopt;
                } else {
                    auto header = buffer.substr(headerStart, headerName);
                    std::transform(header.begin(), header.end(), header.begin(),
                                   [](unsigned char c) { return std::tolower(c); });

                    if (header == "content-length") {
                        char* processed;
                        auto contentLen = std::strtol(buffer.c_str() + headerStart + 2, &processed, 10);
                        if (!contentLen || errno == ERANGE) {
                            return std::nullopt;
                        }
                        parsedRequest->setContentLength(contentLen);
                    } else {
                        auto headerValue = buffer.substr(headerName + 2, nextEnd);
                        parsedRequest->setHeader(std::move(header), std::move(headerValue));
                    }
                }
            }
        }
    }
}

