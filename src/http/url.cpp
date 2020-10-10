#include "http/url.h"
#include <cerrno>

using namespace http;

std::optional<url> url::parse(std::string url) {
    auto idx = url.find_first_of(':');
    if (idx > (unsigned long)std::numeric_limits<std::uint8_t>::max()) {
        return std::nullopt;
    }

    const auto schemeIdx = static_cast<const std::uint8_t>(idx);

    auto endHost = url.find_first_of(':', idx + 3);
    bool hasPort = true;
    if (endHost == std::string::npos) {
        hasPort = false;
        endHost = url.find_first_of('/', idx + 3);
        if (endHost == std::string::npos) {
            endHost = url.size();
        }
    }

    const std::pair<std::uint16_t, std::uint16_t> hostRange{idx + 3, endHost};
    idx = endHost;

    std::uint16_t port = 0;
    if (hasPort) {
        errno = 0;
        char* processed;
        auto iPort = std::strtol(url.c_str() + idx + 1, &processed, 10);
        if (!iPort || errno == ERANGE || iPort > std::numeric_limits<std::uint16_t>::max()) {
            return std::nullopt;
        }

        idx = processed - url.c_str();
        port = iPort;
    }

    std::pair<std::uint16_t, std::uint16_t> pathRange;
    if (idx == url.size()) {
        pathRange.first = url.size();
        pathRange.second = url.size();
    } else {
        auto query = url.find_first_of('?', idx);

        pathRange.first = idx + 1;
        pathRange.second = query == std::string::npos ? url.size() : query;
    }

    return { { url, schemeIdx, hostRange, port, pathRange } };
}
