//
// Created by Luis Holanda on 10/10/20.
//

#include "http/request.h"

using namespace http;

void request::sendHead() {
    std::string buffer;
    buffer.reserve(1024);

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
        case method::Options:
            buffer += "OPTIONS ";
            break;
        case method::Trace:
            buffer += "TRACE ";
            break;
        case method::Connect:
            buffer += "CONNECT ";
            break;
        default:
            break;
    }

    buffer += getUrl().getFullUrl();
    buffer += " HTTP/1.0\r\n";

    auto headers = getHeaders();
    for (auto& h : headers) {
        buffer += h.first;
        buffer += ':';
        buffer += h.second;
        buffer += "\r\n";
    }

    buffer += "\r\n";
}

