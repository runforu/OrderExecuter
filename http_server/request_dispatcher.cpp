//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "mime_types.h"
#include "reply.h"
#include "request.h"
#include "request_dispatcher.h"
#include "request_handler.h"
#include "request_handler_provider.h"

namespace http {
namespace server {

request_dispatcher::request_dispatcher() {}

void request_dispatcher::dispatch_request(const request& req, reply& rep) {
    for (auto item : provider->get_handlers()) {
        if (item->handle(req, rep)) {
            return;
        }
    }
    // no handler found
    return;
}

void request_dispatcher::set_request_handler_provider(request_handler_provider* handlers) {
    provider = handlers;
}

}  // namespace server
}  // namespace http
