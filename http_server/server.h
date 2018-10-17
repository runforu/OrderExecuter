//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _HTTP_SERVER_SERVER_H_
#define _HTTP_SERVER_SERVER_H_

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include "connection.h"
#include "request_dispatcher.h"
#include "request_handler.h"
#include "request_handler_provider.h"

namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server : private boost::noncopyable {
public:
    /// Handle a request to stop the server.
    void stop();

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit server(const std::string& address, const std::string& port, const std::string& doc_root,
                    std::size_t thread_pool_size, request_handler_provider* handlers);

    /// Run the server's io_context loop.
    void run();

private:  
    /// Initiate an asynchronous accept operation.
    void start_accept();

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code& e);

    /// The number of threads that will call io_context::run().
    std::size_t thread_pool_size_;

    /// The io_context used to perform asynchronous operations.
    boost::asio::io_context io_context_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The next connection to be accepted.
    connection_ptr new_connection_;

    /// The handler for all incoming requests.
    request_dispatcher request_dispatcher_;
};

}  // namespace server
}  // namespace http

#endif  // _HTTP_SERVER_SERVER_H_
