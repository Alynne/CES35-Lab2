//
// Created by Luis Holanda on 10/10/20.
//

#include "http/object.h"
#include "arpa/inet.h"
#include <algorithm>
#include <string>
#include <cctype>
#include <cerrno>
#include <iostream>

using namespace http;

namespace http {
    std::string_view
    substringView(const std::string &buffer, size_t start, size_t len) {
        if (start > buffer.size()) return {};
        return std::string_view{buffer.c_str() + start,
                                std::min(buffer.size() - start, len)};
    }
}

size_t object::recvBody(bytes &buffer) {
    errno = 0;
    if (mRemainingLength == 0) {
        return 0;
    }

    auto readSize = std::min(mRemainingLength, (std::uint64_t) buffer.capacity());
    buffer.resize(readSize);
    auto bytesRead = recvFromSock(mSocket, (void*) buffer.c_str(), readSize);
    buffer.resize(bytesRead);

    switch (errno) {
        case ETIMEDOUT:
        case ECONNRESET:
            mConnected = false;
            break;
        default:
            break;
    }

    consumeBody(bytesRead);

    return bytesRead;
}

void object::sendBodyPart(const bytes &buffer) {
    std::size_t bytesSent = 0;
    std::cout << "buffer size: " << buffer.size() << std::endl;
    while (bytesSent != buffer.size()) {
        auto sent = send(mSocket, buffer.data() + bytesSent, buffer.size() - bytesSent,
                         0);
        if (sent == -1) {
            switch (errno) {
                case EACCES:
                    throw std::runtime_error(
                            "tried to broadcast in a non-broadcast socket");
                case EAGAIN:
                    throw std::runtime_error(
                            "received socket is configured to asynchronous receive");
                case EBADF:
                case ENOTSOCK:
                    throw std::runtime_error(
                            "received socket is an invalid file descriptor");
                case ETIMEDOUT:
                case ECONNRESET:
                case EHOSTUNREACH:
                case ENETDOWN:
                case ENETUNREACH:
                case EPIPE:
                    mConnected = false;
                    break;
                case ENOTCONN:
                    throw std::logic_error("the socket isn't connected");
                case EFAULT:
                    std::exit(11);
                case EINVAL:
                case EINTR:
                case ENOBUFS:
                    continue;
                case EOPNOTSUPP:
                    throw std::runtime_error(
                            "the operation isn't supported in the socket");
                case EDESTADDRREQ:
                    throw std::runtime_error("the socket has no peer-address set");
                default:
                    break;
            }
        } else {
            bytesSent += sent;
        }
        std::cout << "sent: " << bytesSent << " bytes" << std::endl; 
    }
}

void object::sendHead() {
    bytes buffer;
    buffer.reserve(1024);

    writeStartLine(buffer);
    writeHeaders(buffer);

    buffer += "\r\n";
    std::cout << "result buffer:" << buffer;
    
    sendBodyPart(buffer);
}

void object::writeHeaders(bytes &buffer) const noexcept {
    buffer += "content-length: ";
    buffer += std::to_string(mContentLength);
    buffer += "\r\n";

    for (const auto& h: getHeaders()) {
        buffer += h.first;
        buffer += ": ";
        buffer += h.second;
        buffer += "\r\n";
    }
}

void object::setHeader(std::string header, std::string value) noexcept {
    std::transform(header.begin(), header.end(), header.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (header != "content-length") {
        mHeaders[std::move(header)] = std::move(value);
    }
}

size_t object::recvFromSock(int socket, void* buffer, int size) {
    size_t bytesRead;
    while (true) {
        std::cout << "receiving " << size << " bytes" << std::endl;
        bytesRead = recv(socket, buffer, size, 0);

        if (bytesRead == -1) {
            switch (errno) {
                case EAGAIN:
                    throw std::runtime_error(
                            "received socket is configured to asynchronous receive");
                case EBADF:
                case ENOTSOCK:
                    throw std::runtime_error(
                            "received socket is an invalid file descriptor");
                case EFAULT:
                    std::exit(11);
                case EINVAL:
                case EINTR:
                case ENOBUFS:
                    std::cout << "errno: " << errno << std::endl;
                    continue;
                case ENOTCONN:
                    throw std::logic_error("the socket isn't connected");
                case EOPNOTSUPP:
                    throw std::runtime_error(
                            "the operation isn't supported in the socket");
                default:
                    std::cerr << "oii" << std::endl;
                    break;
                    {}
            }
        } else {
            break;
        }
    }

    return bytesRead;
}

std::optional<bytes> object::parseHeaders(bytes &buffer, std::size_t off) {
    std::size_t headerStart = off;
    off = buffer.size();
    while (true) {
        auto nextEnd = buffer.find_first_of('\r', headerStart);
        if (nextEnd == std::string::npos || (nextEnd <= buffer.size() && nextEnd > buffer.size() - 4)) {
            // not yet received end of line, or the end of line is in the end of the buffer.
            // in either cases, request more bytes to check for the end of line/headers.
            buffer.resize(buffer.size() + 1024);
            auto received = recvFromSock(mSocket, buffer.data() + off, buffer.size() - off);
            if (received != 0 && received != -1) {
                off += received;
            }
        } else {
            auto headerName = buffer.find_first_of(':', headerStart);
            if (headerName == std::string::npos) {
                return std::nullopt;
            } else {
                auto header = buffer.substr(headerStart, headerName - headerStart);
                std::transform(header.begin(), header.end(), header.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                if (header == "content-length") {
                    char* processed = NULL;
                    const char* contentLengthStrBegin = buffer.c_str() + headerName + 2;
                    auto contentLen = std::strtol(contentLengthStrBegin, &processed, 10);
                    if ((contentLen == 0 && processed == contentLengthStrBegin) || errno == ERANGE) {
                        return std::nullopt;
                    }
                    std::cout << "\n<<SETTING HEADER>> \"" << "content-length" << "\" <<TO>> " << contentLen << std::endl;
                    setContentLength(contentLen);
                } else {
                    auto headerValueStart = headerName + 2;
                    auto headerValue = buffer.substr(headerValueStart, nextEnd - headerValueStart);
                    std::cout << "\n<<SETTING HEADER>> \"" << header << "\" <<TO>> " << headerValue << std::endl;
                    setHeader(std::move(header), std::move(headerValue));
                }

                headerStart = nextEnd + 2;
            }

            auto headersFinished = buffer[nextEnd + 1] == '\n'
                                   && buffer[nextEnd + 2] == '\r'
                                   && buffer[nextEnd + 3] == '\n';
            if (headersFinished) {
                std::cout << off << " " << mContentLength << std::endl;
                auto bodyStart = nextEnd + 4;
                auto bodyLen = mContentLength == 0
                               ? (std::uint64_t) off - bodyStart
                               : std::min((std::uint64_t) off - bodyStart, mContentLength);
                std::cout << "body slice: " << bodyStart << " - " << bodyLen << std::endl;
                auto body = buffer.substr(bodyStart, bodyLen);
                consumeBody(bodyLen);
                return {std::move(body)};
            }
        }
    }
}
