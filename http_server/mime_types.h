//
// mime_types.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _HTTP_SERVER_MIME_TYPES_H_
#define _HTTP_SERVER_MIME_TYPES_H_

#include <string>

namespace http {
namespace server {
namespace mime_types {

/// Convert a file extension into a MIME type.
std::string extension_to_type(const std::string& extension);

}  // namespace mime_types
}  // namespace server
}  // namespace http

#endif  // _HTTP_SERVER_MIME_TYPES_H_
