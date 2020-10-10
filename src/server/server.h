
#ifndef CES35LAB2_SERVER_SERVER_H
#define CES35LAB2_SERVER_SERVER_H

#include <string>
#include <http.h>

class http_connection {
public:
    http_request parse();
    void send(http_object response);
};

class http_server {
public:
    http_server(unsigned short port, std::string dir);
    void listen();
};

#endif