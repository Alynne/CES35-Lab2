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
            , object(0ULL, std::move(url), {})
        {
        }

        [[nodiscard]] method getMethod() const noexcept {
            return mMethod;
        }

        void setMethod(method method) noexcept {
            mMethod = method;
        }

        static std::optional<request> parse(int socket);

    protected:
        void writeStartLine(bytes &buffer) const noexcept override;
    private:
        method mMethod;
    };
}


#endif //LAB2_HTTP_REQUEST_H
