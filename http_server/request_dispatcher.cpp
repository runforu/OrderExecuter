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

request_dispatcher::request_dispatcher() : provider(nullptr), interception_(nullptr) {}

void request_dispatcher::dispatch_request(const request& req, reply& rep) {
    if (interception_ != nullptr && interception_->handle(req)) {
        return;
    }

    if (provider == nullptr) {
        return;
    }

    std::vector<request_handler*> handlers;
    for (auto item : provider->get_handlers()) {
        if (item->can_handle(req)) {
            handlers.push_back(item);
        }
    }

    std::sort(handlers.begin(), handlers.end(),
              [](request_handler* left, request_handler* right) { return right->get_priority() < left->get_priority(); });

    for (auto item : handlers) {
        if (item->handle(req, rep)) {
            return;
        }
    }
    if (provider->get_default_handlers()->can_handle(req)) {
        provider->get_default_handlers()->handle(req, rep);
    }
    // no handler found
    return;
}

void request_dispatcher::set_request_handler_provider(request_handler_provider* handlers) {
    provider = handlers;
}

void request_dispatcher::set_request_interception(request_interception* interception) {
    interception_ = interception;
}

}  // namespace server
}  // namespace http
