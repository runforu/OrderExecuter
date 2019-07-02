//
// request_dispatcher.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _HTTP_SERVER_REQUEST_DISPATCHER_H_
#define _HTTP_SERVER_REQUEST_DISPATCHER_H_

#include <boost/noncopyable.hpp>
#include <string>
#include <vector>
#include "request_handler_provider.h"
#include "request_interception.h"

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_dispatcher : private boost::noncopyable {
public:
    /// Construct with a directory containing files to be served.
    explicit request_dispatcher();

    /// Handle a request and produce a reply.
    void dispatch_request(const request& req, reply& rep);

    void set_request_handler_provider(request_handler_provider* provider);

    void set_request_interception(request_interception* interception);

private:
    request_handler_provider* provider;
    request_interception* interception_;
};

}  // namespace server
}  // namespace http

#endif  // _HTTP_SERVER_REQUEST_DISPATCHER_H_
