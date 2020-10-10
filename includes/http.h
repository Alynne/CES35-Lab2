
#ifndef PROJETO2_HTTP_H
#define PROJETO2_HTTP_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

////////////////
// http.h
///////////////

struct url {
public:
    explicit url(std::string url);

    std::string_view host;
    unsigned short port;
    std::string_view path;
private:
    std::string mUrl;
};

class http_object {
public:
    using headers = std::unordered_map<std::string_view, std::string_view>;
    std::uint64_t getContentLength() const;
    const headers& getHeaders() const;
    const url& getURL() const;
    size_t recvBody(std::vector<std::byte> &buffer);

protected:
    http_object(std::uint64_t contentLength, url url, headers headers);
    void initialize(url url, int socket);
};

class http_connection;

class http_request: http_object {
public:
    http_request(url url, int clientSock) 
    :   http_object(0ull, url, clientSock) // TODO: contentLengh is really zero?
    {
        // do parse.
        initialize(url, clientSock);
    }
    static std::optional<http_request> parse(int socket);
protected:


private:
};

class http_response: http_object {
public:
    http_response(http_request request);
    void setStatusCode(unsigned short code) noexcept;
};


#endif //PROJETO2_HTTP_H
