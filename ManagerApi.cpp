#include <iostream>
#include <string.h>
#include <utility>
#include "Config.h"
#include "Environment.h"
#include "ErrorCode.h"
#include "Loger.h"
#include "ManagerApi.h"

char ManagerApiDllPath[256];

ManagerApi& ManagerApi::Instance() {
    static ManagerApi _instance;
    return _instance;
}

void ManagerApi::Initialize(CManagerInterface* man) {
    if (!IsValid()) {
        return;
    }

    int rt = man->IsConnected();
    if (man->IsConnected()) {
        man->Disconnect();
    }

    char server[128] = {0};
    Config::Instance().GetString("server", server, sizeof(server) - 1, "localhost:4433");
    int login = 4;
    Config::Instance().GetInteger("manager", &login, "4");
    char password[16] = {0};
    Config::Instance().GetString("password", password, sizeof(password) - 1, "12345678");

    int res = RET_ERROR;

    if ((res = man->Connect(server)) != RET_OK || (res = man->Login(login, password)) != RET_OK) {
        LOG("Single Initialize error: %s", man->ErrorDescription(res));
        man->Disconnect();
    }
    LOG("Single Initialize success");
}

const ErrorCode* ManagerApi::RequestChart(const char* symbol, int period, int mode, __time32_t start, __time32_t end,
                                          __time32_t timestamp, boost::property_tree::ptree& result) {
    if (!IsValid()) {
        return &ErrorCode::EC_MAN_ERROR;
    }

    CManagerInterface* man = GetInterface();
    int count = 2;
    while (man == NULL) {
        if (count--) {
            return &ErrorCode::EC_MAN_NO_AVAILABLE_INTERFACE;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(16));
        if (!IsValid()) {
            return &ErrorCode::EC_MAN_NO_AVAILABLE_INTERFACE;
        }
        man = GetInterface();
    }

    ChartInfo ci = {0};
    strncpy_s(ci.symbol, symbol, sizeof(ci.symbol));
    ci.period = period;
    ci.mode = mode;
    ci.start = start;
    ci.end = end;
    ci.timesign = timestamp;

    __time32_t timesign;
    int total = 0;
    RateInfo* ri = man->ChartRequest(&ci, &timesign, &total);
    if (ri != NULL) {
        result.put("count", total);
        result.put("timesign", timesign);
        boost::property_tree::ptree rate_infos;
        for (int i = 0; i < total; i++) {
            boost::property_tree::ptree rate_info;
            rate_info.put("open_price", ri[i].open);
            rate_info.put("high", ri[i].high);
            rate_info.put("low", ri[i].low);
            rate_info.put("close", ri[i].close);
            rate_info.put("time", ri[i].ctm);
            rate_info.put("volume", ri[i].vol);
            rate_infos.push_back(std::make_pair("", rate_info));
        }
        result.add_child("rate_infos", rate_infos);
        man->MemFree(ri);
        LOG("ManagerApi::RequestChart complete.");
        PutInterface(man);
        return &ErrorCode::EC_OK;
    } else {
        PutInterface(man);
        return &ErrorCode::EC_MAN_ERROR;
    }
}

CManagerInterface* ManagerApi::GetInterface() {
    if (!IsValid()) {
        return NULL;
    }

    m_synchronizer.Lock();
    if (!m_available_managers.empty()) {
        // use existing interface
        CManagerInterface* man = m_available_managers.back();
        m_available_managers.pop_back();
        m_synchronizer.Unlock();
        if (!man->IsConnected()) {
            Initialize(man);
        }
        return man;
    } else {
        m_synchronizer.Unlock();
        return NULL;
    }
}

void ManagerApi::PutInterface(CManagerInterface* manager) {
    m_synchronizer.Lock();
    m_available_managers.push_back(manager);
    m_synchronizer.Unlock();
}

void ManagerApi::StartHeartBeat() {
    if (!IsValid()) {
        return;
    }

    for (;;) {
        if (!m_running) {
            break;
        }
        // try to exit quickly
        for (int i = 0; i < 200; i++) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            if (!m_running) {
                return;
            }
        }
        // vector
        int availabe_connnection = 0;
        for (std::vector<CManagerInterface*>::iterator it = m_managers.begin(); it != m_managers.end(); ++it) {
            if ((*it) != NULL && (*it)->IsConnected()) {
                int rt = (*it)->Ping();
                availabe_connnection++;
            }
        }
        LOG("ManagerApi available connection (%d)", availabe_connnection);
    }
}

inline ManagerApi::ManagerApi() : m_running(1), m_factory() {
    char buf[256] = {0};
    strncpy(buf, Environment::s_module_path, sizeof(buf));
    strcat(buf, "mtmanapi.moa");
    m_factory.Init(buf);

    if (!m_factory.IsValid()) {
        LOG("ManagerApi::ManagerApi lib is not loaded.");
        return;
    }
    m_factory.WinsockStartup();

    m_managers.reserve(MAX_MANAGER_LOGIN);
    m_available_managers.reserve(MAX_MANAGER_LOGIN);

    // init m_managers.
    while (m_managers.size() < MAX_MANAGER_LOGIN) {
        CManagerInterface* in = NULL;
        if ((in = m_factory.Create(ManAPIVersion)) != NULL) {
            m_managers.push_back(in);
            m_available_managers.push_back(in);
        }
    }

    m_thread = boost::thread(boost::bind(&ManagerApi::StartHeartBeat, this));
}

inline ManagerApi::~ManagerApi() {
    Stop();
    m_thread.join();

    for (std::vector<CManagerInterface*>::iterator it = m_managers.begin(); it != m_managers.end(); ++it) {
        if ((*it) != NULL) {
            if ((*it)->IsConnected()) {
                (*it)->Disconnect();
            }
            (*it)->Release();
            (*it) = NULL;
        }
    }
    m_factory.WinsockCleanup();
    LOG("ManagerApi::~ManagerApi()");
}
