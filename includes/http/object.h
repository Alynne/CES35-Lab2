//
// Created by Luis Holanda on 10/10/20.
//

#ifndef CES35_LAB2_OBJECT_H
#define CES35_LAB2_OBJECT_H

#include <unordered_map>
#include <string_view>
#include "url.h"

namespace http {
    class object {
    public:
        using headers = std::unordered_map<std::string_view, std::string_view>;
        std::uint64_t getContentLength() const;
        const headers& getHeaders() const;
        size_t recvBody(std::vector<std::byte> &buffer);

    protected:
        object(std::uint64_t contentLength, url url, headers headers);
        void initialize(int socket);
    };
}


#endif //CES35_LAB2_OBJECT_H
