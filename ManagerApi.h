#ifndef _MANAGERAPI_H_
#define _MANAGERAPI_H_

#include <boost/property_tree/ptree.hpp>
#include <thread>
#include <vector>
#include "Synchronizer.h"
#include "../include/MT4ManagerAPI.h"

struct ErrorCode;

class ManagerApi {
public:
    static ManagerApi &Instance();

    // for performance and low memory footprint
    void RequestChart(int login, const char *symbol, int period, int mode, __time32_t start, __time32_t end,
                      __time32_t *timestamp, std::string &json_str);

private:
    bool IsValid() {
        return m_factory.IsValid() && m_running;
    }

    inline void Stop() {
        InterlockedExchange(&m_running, 0);
        LOG("ManagerApi::Stop()");
    }

    CManagerInterface *GetInterface();

    void PutInterface(CManagerInterface *manager);

    void Initialize(CManagerInterface *in);

    void StartHeartBeat();

    ManagerApi();

    ~ManagerApi();

    ManagerApi(ManagerApi const &): m_running(0) {}

    void operator=(ManagerApi const &) {}

    void ErrorCodeToString(const ErrorCode *ec, std::string &str);

private:
    CManagerFactory m_factory;
    std::vector<CManagerInterface *> m_available_managers;
    std::vector<CManagerInterface *> m_managers;
    long m_running;
    Synchronizer m_synchronizer;
    std::thread m_thread;

    static const int MAX_MANAGER_LOGIN = 32;
};

#endif  // !_MANAGERAPI_H_