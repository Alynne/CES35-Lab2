
#ifndef PROJETO2_HTTP_H
#define PROJETO2_HTTP_H

#include "http/object.h"
#include "http/request.h"
#include "http/response.h"
#include "http/url.h"

namespace http {
    std::string_view substringView(const std::string &buffer, size_t start, size_t len);
}


#endif //PROJETO2_HTTP_H
