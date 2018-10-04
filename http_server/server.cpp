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
#include "server_property.h"

namespace http {
namespace server {

server::server(const std::string& address, const std::string& port, const std::string& doc_root, std::size_t thread_pool_size,
               request_handler_provider* handlers)
    : thread_pool_size_(thread_pool_size),
      signals_(io_context_),
      acceptor_(io_context_),
      new_connection_(),
      request_dispatcher_() {
#if 0
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif  // defined(SIGQUIT)
    signals_.async_wait(boost::bind(&server::handle_stop, this));
#endif

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_context_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    server_property::s_doc_root = doc_root;

    request_dispatcher_.set_request_handler_provider(handlers);

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

void server::stop() {
    io_context_.stop();
}

void server::start_accept() {
    new_connection_.reset(new connection(io_context_, request_dispatcher_));
    acceptor_.async_accept(new_connection_->socket(),
                           boost::bind(&server::handle_accept, this, boost::asio::placeholders::error));
}

void server::handle_accept(const boost::system::error_code& e) {
    if (!e) {
        new_connection_->start();
    }

    start_accept();
}

void server::handle_stop() {
    io_context_.stop();
}

}  // namespace server
}  // namespace http
