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

    void Initialize();

    void Shutdown();

private:
    Processor() {}

    ~Processor() = default;

    Processor(Processor const&) = delete;

    void operator=(Processor const&) = delete;

    void StartServer(const char* port, const char* num_threads);

private:
    char m_server_port[8];
    char m_max_thread[8];

    http::server::server* m_http_server = nullptr;
    boost::thread m_thread;
    // Synchronizer m_synchronizer;
};

#endif  // !_PROCESSOR_H_