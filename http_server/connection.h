//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _HTTP_SERVER_CONNECTION_H_
#define _HTTP_SERVER_CONNECTION_H_

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "reply.h"
#include "request.h"
#include "request_dispatcher.h"
#include "request_parser.h"

namespace http {
namespace server {

/// Represents a single connection from a client.
class connection : public boost::enable_shared_from_this<connection>, private boost::noncopyable {
public:
    /// Construct a connection with the given io_context.
    explicit connection(boost::asio::io_context& io_context, request_dispatcher& handler);

    /// Get the socket associated with the connection.
    boost::asio::ip::tcp::socket& socket();

    /// Start the first asynchronous operation for the connection.
    void start();

private:
    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code& e);

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_context::strand strand_;

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The handler used to process the incoming request.
    request_dispatcher& request_dispatcher_;

    /// Buffer for incoming data.
    boost::array<char, 2048> buffer_;

    /// The incoming request.
    request request_;

    /// The parser for the incoming request.
    request_parser request_parser_;

    /// The reply to be sent back to the client.
    reply reply_;
};

typedef boost::shared_ptr<connection> connection_ptr;

}  // namespace server
}  // namespace http

#endif  // _HTTP_SERVER_CONNECTION_H_
