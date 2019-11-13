#ifndef _MANAGERAPI_H_
#define _MANAGERAPI_H_

#include <boost/thread.hpp>
#include <vector>
#include "Synchronizer.h"

struct ErrorCode;
class CManagerInterface;
class CManagerFactory;

class ManagerApi {
public:
    static ManagerApi &Instance();

    // for performance and low memory footprint
    void RequestChart(int login, const char *symbol, int period, std::string &mode_str, __time32_t start, __time32_t end,
                      __time32_t *timestamp, std::string &json_str);

private:
    bool IsValid();

    inline void Stop() {
        InterlockedExchange(&m_running, 0);
    }

    CManagerInterface *GetInterface();

    void PutInterface(CManagerInterface *manager);

    void Initialize(CManagerInterface *in);

    void StartHeartBeat();

    ManagerApi();

    ~ManagerApi();

    ManagerApi(ManagerApi const &) = delete;

    void operator=(ManagerApi const &) = delete;

    void ErrorCodeToString(const ErrorCode *ec, std::string &str);

private:
    CManagerFactory *m_factory;
    std::vector<CManagerInterface *> m_available_managers;
    std::vector<CManagerInterface *> m_managers;
    volatile long m_running;
    Synchronizer m_synchronizer;
    boost::thread m_thread;

    static const int MAX_MANAGER_LOGIN = 32;
};

#endif  // !_MANAGERAPI_H_