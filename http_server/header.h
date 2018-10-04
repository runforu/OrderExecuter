//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _HTTP_SERVER_HEADER_H_
#define _HTTP_SERVER_HEADER_H_

#include <string>

namespace http {
namespace server {

class header {
public:
    header() {}
    header(std::string n, std::string v) : name(n), value(v) {}
    header(const char* n, const char* v) : name(n), value(v) {}
    
    bool operator==(const header& h) const {
        return this->name == h.name && this->value == h.value;
    }

public:
    static const header content_type;

    std::string name;
    std::string value;
};

}  // namespace server
}  // namespace http

#endif  // _HTTP_SERVER_HEADER_H_
