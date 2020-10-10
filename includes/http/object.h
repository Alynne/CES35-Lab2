//
// Created by Luis Holanda on 10/10/20.
//

#ifndef CES35_LAB2_OBJECT_H
#define CES35_LAB2_OBJECT_H

#include <unordered_map>
#include <string_view>
#include <vector>
#include "url.h"

namespace http {
    class object {
    public:
        using headers = std::unordered_map<std::string, std::string>;

        [[nodiscard]] bool isConnected() const noexcept {
            return mConnected;
        }


        [[nodiscard]] std::uint64_t getContentLength() const noexcept {
            return mContentLength;
        }

        [[nodiscard]] std::uint64_t getRemainingLength() const noexcept {
            return mRemainingLength;
        }

        [[nodiscard]] const url& getUrl() const noexcept {
            return mUrl;
        }

        [[nodiscard]]
        std::optional<std::string_view> getHeader(const std::string& header) const noexcept {
            try {
                return { mHeaders.at(header) };
            } catch (std::out_of_range& _) {
                return std::nullopt;
            }
        }

        void setHeader(std::string header, std::string value) noexcept {
            mHeaders[std::move(header)] = std::move(value);
        }

        size_t recvBody(std::vector<std::byte> &buffer);
        void sendBodyPart(const std::vector<std::byte> &buffer);

    protected:
        object(std::uint64_t contentLength, url url, headers headers)
            : mContentLength(contentLength)
            , mUrl(std::move(url))
            , mRemainingLength(contentLength)
            , mHeaders(std::move(headers))
        {}

        void initialize(int socket) noexcept {
            mSocket = socket;
        }
    private:
        int mSocket = -1;
        bool mConnected = true;
        const url mUrl;
        const std::uint64_t mContentLength;
        std::uint64_t mRemainingLength;
        headers mHeaders;
    };
}


#endif //CES35_LAB2_OBJECT_H
