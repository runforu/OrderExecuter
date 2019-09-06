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
#include "../Loger.h"

namespace http {
namespace server {
namespace placeholders = boost::asio::placeholders;

int connection::connection_number_ = 0;

int DiffTime(SYSTEMTIME time1, SYSTEMTIME time2) {
    return (time1.wSecond - time2.wSecond) * 1000 + time1.wMilliseconds - time2.wMilliseconds +
           ((time1.wMinute != time2.wMinute) ? 60 * 1000 : 0);
}

connection::connection(boost::asio::io_context& io_context, request_dispatcher& dispatcher)
    : strand_(boost::asio::make_strand(io_context)), socket_(strand_), dispatcher_(dispatcher), timer_(strand_) {
    connection_number_++;
}

boost::asio::ip::tcp::socket& connection::socket() {
    return socket_;
}

void connection::start() {
    try {
        socket_.set_option(boost::asio::ip::tcp::no_delay(true));
        socket_.set_option(boost::asio::socket_base::do_not_route(true));
        socket_.set_option(boost::asio::socket_base::keep_alive(true));
    } catch (...) {
        LOG_LINE;
    }
    do_start();
}

connection::~connection() {
    connection_number_--;
}

int connection::total_connection() {
    return connection_number_;
}

void connection::do_start() {
    try {
        request_parser_.reset();
        request_.reset();
        reply_.reset();
        start_timer();
        boost::asio::async_read(
            socket_, boost::asio::buffer(buffer_), boost::asio::transfer_at_least(1),
            boost::bind(&connection::handle_read, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
    } catch (...) {
        LOG_LINE;
    }
}

void connection::handle_close(const boost::system::error_code& error) {
    if (!error) {
        // Initiate graceful connection closure.
        try {
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            socket_.close();
        } catch (...) {
            LOG_LINE;
        }
    }

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

void connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred) {
    cancel_timer();

    if (!e) {
        try {
            GetLocalTime(&timestamp_);
            boost::tribool result;
            decltype(buffer_.data()) iter;

            boost::tie(result, iter) = request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);

            if (result) {
                dispatcher_.dispatch_request(request_, reply_);

                SYSTEMTIME mt4_end;
                GetLocalTime(&mt4_end);
                mt4_time_ = DiffTime(mt4_end, timestamp_);

                boost::asio::async_write(socket_, reply_.to_buffers(),
                                         boost::bind(&connection::handle_write, shared_from_this(), placeholders::error));
            } else if (!result) {
                reply_ = reply::stock_reply(reply::bad_request);
                boost::asio::async_write(socket_, reply_.to_buffers(),
                                         boost::bind(&connection::handle_write, shared_from_this(), placeholders::error));
            } else {
                boost::asio::async_read(socket_, boost::asio::buffer(buffer_), boost::asio::transfer_at_least(1),
                                        boost::bind(&connection::handle_read, shared_from_this(), placeholders::error,
                                                    placeholders::bytes_transferred));
            }
        } catch (...) {
            LOG_LINE;
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.
}

void connection::handle_write(const boost::system::error_code& e) {
    if (!e) {
        try {
            if (request_.body.length() > 10) {
                std::size_t begin = request_.body.find("request");
                begin = request_.body.find(":", begin);
                begin = request_.body.find("\"", begin) + 1;
                std::size_t end = request_.body.find("\"", begin);
                if (end > begin && begin > 0) {
                    SYSTEMTIME write_end;
                    GetLocalTime(&write_end);
                    LOG("Request [%s] takes %d, mt4 takes %d", request_.body.substr(begin, end - begin).c_str(),
                        DiffTime(write_end, timestamp_), mt4_time_);
                }
            }
        } catch (...) {
            LOG_LINE;
        }
        // keep the connection alive.
        do_start();
    }

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
}

void connection::start_timer() {
    try {
        // asynchronized handler will be cancelled.
        timer_.expires_from_now(boost::posix_time::seconds(200));
        timer_.async_wait(boost::bind(&connection::handle_close, shared_from_this(), placeholders::error));
    } catch (...) {
        LOG_LINE;
    }
}

void connection::cancel_timer() {
    try {
        timer_.cancel();
    } catch (...) {
        LOG_LINE;
    }
}

}  // namespace server
}  // namespace http
