//
// Created by Luis Holanda on 10/10/20.
//

#ifndef LAB2_HTTP_REQUEST_H
#define LAB2_HTTP_REQUEST_H


#include "object.h"
#include "url.h"

namespace http {
    class request: object {
    public:
        request(url url, int socket);

        static std::optional<request> parse(int socket);
    };
}


#endif //LAB2_HTTP_REQUEST_H
