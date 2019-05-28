#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <process.h>
#include <stdlib.h>
#include "Config.h"
#include "LicenseService.h"
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

void Processor::Initialize() {
    FUNC_WARDER;

    // Init the http server settings
    Config::Instance().GetString("Http.Server.port!reboot", m_server_port, sizeof(m_server_port) - 1, "8080");
    Config::Instance().GetString("Max.Http.Threads!reboot", m_max_thread, sizeof(m_max_thread) - 1, "512");
    LOG("Processor::Initialize %s, %s", m_server_port, m_max_thread);
    m_thread = boost::thread(boost::bind(&Processor::StartServer, this, m_server_port, m_max_thread));

#ifdef _LICENSE_VERIFICATION_
    LicenseService::Instance().ResetLicense();
#endif  // !_LICENSE_VERIFICATION_
}

void Processor::Shutdown() {

#ifdef _LICENSE_VERIFICATION_
    LicenseService::Instance().Stop();
#endif  // !_LICENSE_VERIFICATION_

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
