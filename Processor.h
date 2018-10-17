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
    void Initialize(const char* port, const char* root, const char* num_threads);
    void Shutdown();
    Processor();

private:
    void StartServer(const char* port, const char* root, const char* num_threads);

private:
    http::server::server* m_http_server;
    boost::thread m_thread;
};

#endif  // !_PROCESSOR_H_