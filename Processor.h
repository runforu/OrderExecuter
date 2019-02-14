#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <boost/thread.hpp>

namespace http {
namespace server {
class server;
}
}  // namespace http

class Processor {
    friend class Factory;

public:
    static Processor& Instance();

    void Initialize(const char* port, const char* num_threads);
    void Shutdown();

private:
    Processor() : m_http_server(NULL) {}
    ~Processor(){};
    Processor(Processor const&) {}
    void operator=(Processor const&) {}

    void StartServer(const char* port, const char* num_threads);

private:
    http::server::server* m_http_server;
    boost::thread m_thread;
    // Synchronizer m_synchronizer;
};

#endif  // !_PROCESSOR_H_