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

