//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <algorithm>
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

request_dispatcher::request_dispatcher(request_handler_provider* provider, request_interception* interception)
    : provider_(provider), interception_(interception) {}

void request_dispatcher::dispatch_request(const request& req, reply& rep) const {
    if (interception_ != nullptr && interception_->handle(req)) {
        return;
    }

    if (provider_ == nullptr) {
        return;
    }

    for (auto item : provider_->get_handlers()) {
        if (item->can_handle(req) && item->handle(req, rep)) {
            return;
        }
    }

    if (provider_->get_default_handlers()->can_handle(req)) {
        provider_->get_default_handlers()->handle(req, rep);
    }

    // no handler found
}

}  // namespace server
}  // namespace http
