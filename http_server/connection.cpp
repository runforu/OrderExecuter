//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <vector>
#include "connection.h"
#include "request_dispatcher.h"

namespace http {
namespace server {

connection::connection(boost::asio::io_context& io_context, request_dispatcher& dispatcher)
    : strand_(io_context), socket_(io_context), dispatcher_(dispatcher), timer_(io_context), timeout_(false) {}

boost::asio::ip::tcp::socket& connection::socket() {
    return socket_;
}

void connection::start() {
    socket_.set_option(boost::asio::ip::tcp::no_delay(true));
    socket_.set_option(boost::asio::socket_base::do_not_route(true));
    do_start();
}

void connection::do_start() {
    request_parser_.reset();
    request_.reset();
    reply_.reset();
    boost::asio::async_read(socket_, boost::asio::buffer(buffer_), boost::asio::transfer_at_least(1),
                            boost::asio::bind_executor(strand_, boost::bind(&connection::handle_read, shared_from_this(),
                                                                            boost::asio::placeholders::error,
                                                                            boost::asio::placeholders::bytes_transferred)));
    try {
        // asynchronized handler will be cancelled.
        timer_.expires_from_now(boost::posix_time::seconds(30));
    } catch (...) {
    }
    timer_.async_wait(boost::asio::bind_executor(
        strand_, boost::bind(&connection::handle_close, shared_from_this(), boost::asio::placeholders::error)));
}

void connection::handle_close(const boost::system::error_code& error) {
    if (!error) {
        timeout_ = true;
    }
    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

void connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred) {
    try {
        timer_.cancel();
    } catch (...) {
    }
    if (!e) {
        boost::tribool result;
        decltype(buffer_.data()) iter;
        boost::tie(result, iter) = request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);

        if (result) {
            dispatcher_.dispatch_request(request_, reply_);
            boost::asio::async_write(
                socket_, reply_.to_buffers(),
                boost::asio::bind_executor(
                    strand_, boost::bind(&connection::handle_write, shared_from_this(), boost::asio::placeholders::error)));
        } else if (!result) {
            reply_ = reply::stock_reply(reply::bad_request);
            boost::asio::async_write(
                socket_, reply_.to_buffers(),
                boost::asio::bind_executor(
                    strand_, boost::bind(&connection::handle_write, shared_from_this(), boost::asio::placeholders::error)));
        } else {
            boost::asio::async_read(
                socket_, boost::asio::buffer(buffer_), boost::asio::transfer_at_least(1),
                boost::asio::bind_executor(
                    strand_, boost::bind(&connection::handle_read, shared_from_this(), boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred)));
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.
}

void connection::handle_write(const boost::system::error_code& e) {
    try {
        timer_.cancel();
    } catch (...) {
    }
    if (!e) {
        if (timeout_) {
            // Initiate graceful connection closure.
            boost::system::error_code ignored_ec;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
            socket_.close();
        } else {
            // keep the connection alive.
            do_start();
        }
    }

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

}  // namespace server
}  // namespace http
