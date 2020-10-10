
#ifndef PROJETO2_HTTP_H
#define PROJETO2_HTTP_H

////////////////
// http.h
///////////////

struct url {
public:
    explicit url(std::string url);

    std::string_view host const;
    unsigned short port const;
    std::string_view path const;
private:
    std::string mUrl;
};

class http_object {
public:
    using headers = std::unordered_map<std::string_view, std::string_view>;
    std::uint64_t getContentLength() const;
    const & getHeaders() const;
    const url& getURL() const;
    size_t recvBody(std::vector<std::byte> &buffer);

protected:
    http_object(std::uint64_t contentLength, url url, headers headers);
    initialize(url url, int socket);
}

class http_connection;

class http_request: http_object {
public:
    http_request(url url, int clientSock) {
        // do parse.
        initialize(url, clientSock)
    }
    static std::optional<http_request> parse(int socket);
protected:


private:
}

class http_response: http_object {
public:
    http_response(http_request request);
    void setStatusCode(unsigned short code) noexcept;
}

////////////////
// client.h
////////////////

class http_client {
    http_client();
    http_response send(const http_request& request);
};

/// <METODO> URL
/// Content-Length: 0
/// ...
/// ...
/// .
/// .
/// .
///
/// <body: stream>

////////////////
// server.h
////////////////

class http_connection {
public:
    http_request parse();
    void send(http_object response);
}

class http_server {
public:
    http_server(unsigned short port, std::string dir);
    void listen();
}

void processRequest(http_object req) {
    std::vector<std::byte> bodyBuffer(4096);
    size_it read;
    while (read = req.recvBody(bodyBuffer)) {
        // process buffer
        fwrite(file, bodyBuffer, read);
    }
}


#endif //PROJETO2_HTTP_H
