//
// Created by Luis Holanda on 10/10/20.
//

#include "http/object.h"
#include "arpa/inet.h"
#include <algorithm>
#include <string>
#include <cctype>
#include <cerrno>

using namespace http;

size_t object::recvBody(bytes &buffer) {
    size_t bytesRead;
    while (true) {
        bytesRead = recv(mSocket, (void *) buffer.data(), buffer.size(), 0);

        if (bytesRead == -1) {
            switch (errno) {
                case EAGAIN:
                    throw std::runtime_error(
                            "received socket is configured to asynchronous receive");
                case EBADF:
                case ENOTSOCK:
                    throw std::runtime_error(
                            "received socket is an invalid file descriptor");
                case ETIMEDOUT:
                case ECONNRESET:
                    mConnected = false;
                    break;
                case EFAULT:
                    std::exit(11);
                case EINVAL:
                case EINTR:
                case ENOBUFS:
                    continue;
                case ENOTCONN:
                    throw std::logic_error("the socket isn't connected");
                case EOPNOTSUPP:
                    throw std::runtime_error(
                            "the operation isn't supported in the socket");
                default:
                    break;
                    {}
            }
        } else {
            mRemainingLength -= bytesRead;
            break;
        }
    }


    return bytesRead;
}

void object::sendBodyPart(const bytes &buffer) {
    std::size_t bytesSent = 0;

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
    }
}

void object::sendHead() {
    bytes buffer;
    buffer.reserve(1024);

    writeStartLine(buffer);
    writeHeaders(buffer);

    buffer += "\r\n";

    sendBodyPart(buffer);
}

void object::writeHeaders(bytes &buffer) const noexcept {
    if (mContentLength) {
        buffer += "content-length: ";
        buffer += std::to_string(mContentLength);
        buffer += "\r\n";
    }

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
