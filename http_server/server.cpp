//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <vector>
#include "server.h"

namespace http {
namespace server {

server::server(const std::string& address, const std::string& port, std::size_t thread_pool_size,
               request_handler_provider* handlers, request_interception* interception)
    : thread_pool_size_(thread_pool_size), acceptor_(io_context_), new_connection_(), dispatcher_() {
    // Set the requestion handler providers.
    dispatcher_.set_request_handler_provider(handlers);
    dispatcher_.set_request_interception(interception);

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
    std::vector<boost::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < thread_pool_size_; ++i) {
        boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind(&boost::asio::io_context::run, &io_context_)));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i) threads[i]->join();
}

void server::start_accept() {
    new_connection_.reset(new connection(io_context_, dispatcher_));
    acceptor_.async_accept(new_connection_->socket(),
                           boost::bind(&server::handle_accept, this, boost::asio::placeholders::error));
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
