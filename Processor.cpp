#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <process.h>
#include <stdlib.h>
#include "Config.h"
#include "Loger.h"
#include "Processor.h"
#include "RequestHandlerProviderImp.h"
#include "ServerApi.h"
#include "http_server/server.h"
#include "../include/MT4ServerAPI.h"

Processor& Processor::Instance() {
    static Processor _instance;
    return _instance;
}

void Processor::Initialize(const char* port, const char* num_threads) {
    FUNC_WARDER;
    m_thread = boost::thread(boost::bind(&Processor::StartServer, this, port, num_threads));
}

void Processor::Shutdown() {
    if (m_http_server != NULL) {
        m_http_server->stop();
    }
    m_thread.join();
}

void Processor::StartServer(const char* port, const char* num_threads) {
    LOG("http server port: %s, threads: %s", port, num_threads);
    try {
        // Initialise the server.
        RequestHandlerProviderImp provider;
        std::size_t number = boost::lexical_cast<std::size_t>(num_threads);
        m_http_server = new http::server::server("0.0.0.0", port, number, &provider);

        // Run the server until stopped.
        m_http_server->run();
    } catch (std::exception& e) {
        LOG("http server failed: %s", e.what());
    }
    if (m_http_server != 0) {
        delete m_http_server;
        m_http_server = NULL;
    }
    LOG("http server stops.");
}
