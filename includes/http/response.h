//
// Created by Luis Holanda on 10/10/20.
//

#ifndef CES35_LAB2_RESPONSE_H
#define CES35_LAB2_RESPONSE_H

#include "object.h"
#include "request.h"

namespace http {
    class status {
    public:
        enum code: std::uint16_t {
            Ok = 200,
            Created,
            Accepted,
            NoContent = 204,
            MultipleChoices = 300,
            MovedPermanently,
            MovedTemporarily,
            NotModified = 304,
            BadRequest = 400,
            Unauthorized,
            Forbidden = 403,
            NotFound,
            InternalServerError = 500,
            NotImplemented,
            BadGateway,
            ServiceUnavailable
        };

        // Keep constructor implicit as to permit pass code where status is expected.
        status(code code) : mCode(code) {};

        [[nodiscard]] constexpr std::string_view asString() const noexcept {
            switch (mCode) {
                case code::Ok: return "200";
                case code::Created: return "201";
                case code::Accepted: return "202";
                case code::NoContent: return "204";
                case code::MultipleChoices: return "300";
                case code::MovedPermanently: return "301";
                case code::MovedTemporarily: return "302";
                case code::NotModified: return "304";
                case code::BadRequest: return "400";
                case code::Unauthorized: return "401";
                case code::Forbidden: return "403";
                case code::NotFound: return "404";
                case code::InternalServerError: return "500";
                case code::NotImplemented: return "501";
                case code::BadGateway: return "502";
                case code::ServiceUnavailable: return "503";
            }
        }

        [[nodiscard]] constexpr std::string_view reasonPhrase() const noexcept {
            switch (mCode) {
                case code::Ok: return "Ok";
                case code::Created: return "Created";
                case code::Accepted: return "Accepted";
                case code::NoContent: return "No Content";
                case code::MultipleChoices: return "Multiple Choices";
                case code::MovedPermanently: return "Moved Permanently";
                case code::MovedTemporarily: return "Moved Temporarily";
                case code::NotModified: return "Not Modified";
                case code::BadRequest: return "Bad Request";
                case code::Unauthorized: return "Unauthorized";
                case code::Forbidden: return "Forbidden";
                case code::NotFound: return "Not Found";
                case code::InternalServerError: return "Internal Server Error";
                case code::NotImplemented: return "Not Implemented";
                case code::BadGateway: return "Bad Gateway";
                case code::ServiceUnavailable: return "Service Unavailable";
            }
        }

    private:
        code mCode;
    };

    class response : public object {
    public:
        explicit response(status statusCode)
                : object(0ULL, {})
                  , mStatusCode(statusCode)
        {};

        [[nodiscard]] status getStatusCode() const noexcept {
            return mStatusCode;
        }

        void setStatusCode(status code) noexcept {
            mStatusCode = code;
        }

        static std::optional<std::pair<response, bytes>> parse(int socket);
    protected:

        void writeStartLine(bytes &buffer) const noexcept override;
    private:
        status mStatusCode;
    };
}


#endif //CES35_LAB2_RESPONSE_H
