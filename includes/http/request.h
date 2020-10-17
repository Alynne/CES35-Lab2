//
// Created by Luis Holanda on 10/10/20.
//

#ifndef LAB2_HTTP_REQUEST_H
#define LAB2_HTTP_REQUEST_H


#include "object.h"
#include "url.h"

namespace http {
    enum class method {
        Get,
        Post,
        Put,
        Delete,
        Head,
        Link,
        Unlink
    };

    class request: public object {
    public:
        request(method method, url url)
            : mMethod(method)
            , mUrl(std::move(url))
            , object(0ULL, {})
        {
        }

        [[nodiscard]] const url& getUrl() const noexcept {
            return mUrl;
        }

        [[nodiscard]] method getMethod() const noexcept {
            return mMethod;
        }

        void setMethod(method method) noexcept {
            mMethod = method;
        }

        static std::optional<std::pair<request, bytes>> parse(int socket);

    protected:
        void writeStartLine(bytes &buffer) const noexcept override;
    private:
        method mMethod;
        url mUrl;
    };
}


#endif //LAB2_HTTP_REQUEST_H
