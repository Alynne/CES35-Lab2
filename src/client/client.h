#ifndef CES35LAB2_CLIENT_CLIENT_H
#define CES35LAB2_CLIENT_CLIENT_H

#include <http.h>

class http_client {
    http_client();
    http::response send(const http::request& request);
};


// void processRequest(http_object req) {
//     std::vector<std::byte> bodyBuffer(4096);
//     size_t read;
//     while (read = req.recvBody(bodyBuffer)) {
//         // process buffer
//         fwrite(file, bodyBuffer, read);
//     }
// }

#endif
