//
// Created by Luis Holanda on 10/10/20.
//

#include "http.h"
#include <cerrno>
#include <iostream>

using namespace http;

void response::writeStartLine(bytes &buffer) const noexcept {
    buffer += "HTTP/1.0 ";
    buffer += mStatusCode.asString();
    buffer += ' ';
    buffer += mStatusCode.reasonPhrase();
    buffer += "\r\n";
}

static std::optional<status>
parseResponseLine(const std::string &buffer) {
    auto statusStr = substringView(buffer, 8 /* advance HTTP version */ + 1, 3);
    char* processed;
    auto statusInt = std::strtol(statusStr.begin(), &processed, 10);
    if (!statusInt || errno == ERANGE) {
        return std::nullopt;
    }

    switch (statusInt) {
        case status::Ok:
            return status(status::Ok);
        case status::Created:
            return status(status::Created);
        case status::Accepted:
            return status(status::Accepted);
        case status::NoContent:
            return status(status::NoContent);
        case status::MultipleChoices:
            return status(status::MultipleChoices);
        case status::MovedPermanently:
            return status(status::MovedPermanently);
        case status::MovedTemporarily:
            return status(status::MovedTemporarily);
        case status::NotModified:
            return status(status::NotModified);
        case status::BadRequest:
            return status(status::BadRequest);
        case status::Unauthorized:
            return status(status::Unauthorized);
        case status::Forbidden:
            return status(status::Forbidden);
        case status::NotFound:
            return status(status::NotFound);
        case status::InternalServerError:
            return status(status::InternalServerError);
        case status::NotImplemented:
            return status(status::NotImplemented);
        case status::BadGateway:
            return status(status::BadGateway);
        case status::ServiceUnavailable:
            return status(status::ServiceUnavailable);
        default:
            return std::nullopt;
    }
};

std::optional<std::pair<response, bytes>> response::parse(int socket) {
    bytes buffer;
    buffer.resize(4096);
    std::size_t off = 0;
    std::optional<response> parsedResponse;

    // Parse request line.
    while (true) {
        auto received = recvFromSock(socket, buffer.data() + off, buffer.size() - off);
        if (received != 0 && received != -1) {
            // std::cout << buffer << std::endl;
            auto lineEnd = buffer.find_first_of('\r');
            if (lineEnd == std::string::npos) {
                buffer.resize(buffer.size() + 4096);
                off += received;
                continue;
            } else {
                auto responseLine = parseResponseLine(buffer);
                if (!responseLine.has_value()) {
                    return std::nullopt;
                } else {
                    parsedResponse.emplace(response(responseLine.value()));
                    off = lineEnd + 2;
                    break;
                }
            }
        } else if (received == 0) {
            // No more data to receive, but found no '\r\n'
            throw std::runtime_error("data improperly formatted");
        } else {
            throw std::runtime_error("error when receiving header");
        }
    }
    // std::cout << std::endl<< std::endl<< std::endl;

    // Parse headers.
    std::size_t headerStart = off;
    int loop = 0;
    while (true) {
        loop++;
        auto nextEnd = buffer.find_first_of('\r', headerStart);
        if (nextEnd == std::string::npos || (nextEnd <= buffer.size() && nextEnd > buffer.size() - 4)) {
            buffer.resize(buffer.size() + 1024);
            auto received = recvFromSock(socket, buffer.data() + off, buffer.size() - off);
            if (received != 0 && received != -1) {
                off += received;
            }
        } else {
            auto headersFinished = buffer[nextEnd + 1] == '\n'
                                   && buffer[nextEnd + 2] == '\r'
                                   && buffer[nextEnd + 3] == '\n';
            if (headersFinished) {
                auto bodyStart = buffer.substr(nextEnd + 4);
                parsedResponse->consumeBody(buffer.size() - bodyStart.size());
                return {{parsedResponse.value(), std::move(bodyStart)}};
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
                        parsedResponse->setContentLength(contentLen);
                    } else {
                        auto headerValue = buffer.substr(headerName + 2, nextEnd);
                        // std::cout << "\n<<SETTING HEADER>> \"" << header << "\" <<TO>> " << headerValue << std::endl;
                        parsedResponse->setHeader(std::move(header), std::move(headerValue));
                    }
                    headerStart = nextEnd+2;
                }
            }
        }
    }
}
