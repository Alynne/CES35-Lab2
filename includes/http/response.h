//
// Created by Luis Holanda on 10/10/20.
//

#ifndef CES35_LAB2_RESPONSE_H
#define CES35_LAB2_RESPONSE_H

#include "object.h"
#include "request.h"

namespace http {
    class response: public object {
    public:
        response(request request);
        void setStatusCode(std::uint16_t code) noexcept;
    };
}


#endif //CES35_LAB2_RESPONSE_H
