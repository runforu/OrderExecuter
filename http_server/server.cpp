//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include "server.h"

namespace http {
namespace server {

server::server(const std::string& address, const std::string& port, std::size_t thread_pool_size,
               request_handler_provider* handlers, request_interception* interception)
    : thread_pool_size_(thread_pool_size), acceptor_(io_context_), new_connection_(), dispatcher_(handlers, interception) {
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_context_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    start_accept();
}

void server::run() {
    // Create a pool of threads to run all of the io_contexts.
    std::vector<std::thread*> threads;
    for (std::size_t i = 0; i < thread_pool_size_; ++i) {
        threads.push_back(new std::thread(std::bind(
            static_cast<boost::asio::io_context::count_type (boost::asio::io_context::*)()>(&boost::asio::io_context::run),
            &io_context_)));
    }

    // Wait for all threads in the pool to exit.
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        (*it)->join();
    }
}

void server::start_accept() {
    try {
        new_connection_.reset(new connection(io_context_, dispatcher_));
        // acceptor_.async_accept(new_connection_->socket(), std::bind(&server::handle_accept, this, std::placeholders::_1));
        acceptor_.async_accept(new_connection_->socket(),
                               [this](const boost::system::error_code& error) -> void { this->handle_accept(error); });
    } catch (...) {
    }
}

void server::handle_accept(const boost::system::error_code& e) {
    if (!e) {
        new_connection_->start();
    }

    start_accept();
}

void server::stop() {
    io_context_.stop();
}

}  // namespace server
}  // namespace http
