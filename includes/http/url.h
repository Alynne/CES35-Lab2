//
// Created by Luis Holanda on 10/10/20.
//

#ifndef LAB2_URL_H
#define LAB2_URL_H

#include <optional>
#include <string>

namespace http {
    /// A URL as specified by the RFC 1738.
    struct url {
    public:
        /// The port specified in the URL.
        const std::uint16_t port;

        /// Get a reference to the scheme of the URL.
        [[nodiscard]] std::string_view getScheme() const noexcept {
            return {mUrl.c_str(), static_cast<unsigned long>(mScheme)};
        }

        /// Get a reference to the host of the URL.
        [[nodiscard]] std::string_view getHost() const noexcept {
            return {mUrl.c_str() + mHost.first, mHost.second};
        }

        /// Get a reference to the path of the URL.
        [[nodiscard]] std::string_view getPath() const noexcept {
            return {mUrl.c_str() + mPath.first, mPath.second};
        }

        /// Get a reference to the full URL string.
        [[nodiscard]] std::string_view getFullUrl() const noexcept {
            return mUrl;
        }

        /// Parses a string into a URL, if possible.
        static std::optional<url> parse(std::string url);

    private:
        url(std::string url, std::uint8_t scheme,
            std::pair<std::uint16_t, std::uint16_t> host, std::uint16_t port,
            std::pair<std::uint16_t, std::uint16_t> path) noexcept
                : mUrl(std::move(url))
                  , mScheme(scheme)
                  , port(port)
                  , mHost(host.first, host.second - host.first)
                  , mPath(path.first, path.second - path.first) {};

        std::string mUrl;
        std::uint8_t mScheme;
        std::pair<std::uint16_t, std::uint16_t> mHost;
        std::pair<std::uint16_t, std::uint16_t> mPath;
    };
}

#endif //LAB2_URL_H
