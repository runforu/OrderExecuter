#include <iostream>
#include <string>
#include <utility>
#include "Config.h"
#include "Environment.h"
#include "ErrorCode.h"
#include "Loger.h"
#include "ManagerApi.h"
#include "common.h"

ManagerApi& ManagerApi::Instance() {
    static ManagerApi _instance;
    return _instance;
}

void ManagerApi::Initialize(CManagerInterface* man) {
    if (!IsValid()) {
        return;
    }

    if (man->IsConnected()) {
        man->Disconnect();
    }

    char server[128] = {0};
    Config::Instance().GetString("Manager.Api.Server!reboot", server, sizeof(server) - 1, "localhost:4433");
    int login = 4;
    Config::Instance().GetInteger("Manager.Api.Login!reboot", &login, "4");
    char password[16] = {0};
    Config::Instance().GetString("Manager.Api.Password!reboot", password, sizeof(password) - 1, "12345678");
    int res = RET_ERROR;

    if ((res = man->Connect(server)) != RET_OK || (res = man->Login(login, password)) != RET_OK) {
        LOG("Manager interface initialize error: %s", man->ErrorDescription(res));
        man->Disconnect();
    }
    LOG("New manager interface initialized.");
}

void ManagerApi::RequestChart(int login, const char* symbol, int period, int mode, __time32_t start, __time32_t end,
                              __time32_t* timestamp, std::string& json_str) {
    FUNC_WARDER;

    if (!IsValid()) {
        ErrorCodeToString(&ErrorCode::EC_MAN_ERROR, json_str);
        return;
    }

    CManagerInterface* man = GetInterface();
    if (man == NULL) {
        ErrorCodeToString(&ErrorCode::EC_MAN_NO_AVAILABLE_INTERFACE, json_str);
        return;
    }

    ChartInfo ci = {0};
    strncpy_s(ci.symbol, symbol, sizeof(ci.symbol));
    ci.period = period;
    ci.mode = mode;
    ci.start = start;
    ci.end = end;
    ci.timesign = *timestamp;

    void* rate_info = NULL;
    int total = 0;
    RateInfo* ri = man->ChartRequest(&ci, timestamp, &total);
    if (ri != NULL) {
        int spread_diff = 0;
        if (login != -1) {
            spread_diff = GetSpreadDiff(login, symbol);
        }
        // LOG("ManagerApi::RequestChart complete %d.", total);
        // avarage length of a RateInfo json is less than 100, the max length of other info is less than 256
        try {
            json_str.reserve(total * 100 + 256);
            json_str.append("{\"request\":\"RequestChart\",");
            json_str.append("\"result\":\"").append(ErrorCode::EC_OK.m_code == 0 ? "OK" : "ERROR").append("\",");
            json_str.append("\"error_code\":").append(std::to_string(ErrorCode::EC_OK.m_code)).append(",");
            json_str.append("\"error_des\":\"").append(ErrorCode::EC_OK.m_des).append("\",");
            json_str.append("\"count\":").append(std::to_string(total)).append(",");
            json_str.append("\"timesign\":").append(std::to_string(*timestamp)).append(",");
            json_str.append("\"rate_infos\":").append("[");
            for (int i = 0; i < total; i++) {
                json_str.append("{");
                json_str.append("\"open_price\":").append(std::to_string(ri[i].open - spread_diff / 2)).append(",");
                json_str.append("\"high\":").append(std::to_string(ri[i].high)).append(",");
                json_str.append("\"low\":").append(std::to_string(ri[i].low)).append(",");
                json_str.append("\"close\":").append(std::to_string(ri[i].close)).append(",");
                json_str.append("\"time\":").append(std::to_string(ri[i].ctm)).append(",");
                json_str.append("\"volume\":").append(std::to_string(ri[i].vol));
                json_str.append("}");
                if (i != total - 1) {
                    json_str.append(",");
                }
            }
            json_str.append("]}\r\n");
        } catch (...) {
            json_str.clear();
            ErrorCodeToString(&ErrorCode::EC_MAN_ERROR, json_str);
            LOG("No memory to perform the action");
        }
        man->MemFree(ri);
        // LOG("memory = %d, capability = %d, size = %d", (sizeof(RateInfo) * total), json_str.capacity(), json_str.size());
    } else {
        ErrorCodeToString(&ErrorCode::EC_MAN_ERROR, json_str);
    }
    PutInterface(man);
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
        // try to exit quickly
        for (int i = 0; i < 200; i++) {
            if (!m_running) {
                return;
            }
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
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
    COPY_STR(buf, Environment::s_module_path);
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

ManagerApi::~ManagerApi() {
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

void ManagerApi::ErrorCodeToString(const ErrorCode* ec, std::string& str) {
    str.reserve(127);
    str.append("{");
    str.append("\"result\":\"").append(ec->m_code == 0 ? "OK" : "ERROR").append("\",");
    str.append("\"error_code\":").append(std::to_string(ec->m_code)).append(",");
    str.append("\"error_des\":\"").append(ec->m_des).append("\"");
    str.append("}\r\n");
}
