#include "Config.h"
#include "Environment.h"
#include "EventCenter.h"
#include "Loger.h"
#include "Processor.h"
#include "ServerApi.h"
#include "../include/MT4ServerAPI.h"

PluginInfo ExtPluginInfo = {"Order Executer", 1, "DH Copyrigh.", {0}};

static char DocRootPath[256];

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID /*lpReserved*/) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            char tmp[256], *cp;
            GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 5);
            //--- set root path
            COPY_STR(DocRootPath, tmp);
            if ((cp = strrchr(DocRootPath, '\\')) != NULL) {
                *(cp + 1) = 0;
                strncpy(Environment::s_module_path, DocRootPath, sizeof(DocRootPath));
                strcat(DocRootPath, "root\\");
            }
            //--- create configuration filename
            if ((cp = strrchr(tmp, '.')) != NULL) {
                *cp = 0;
                strcat(tmp, ".ini");
            }
            Config::Instance().Load(tmp);
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return (TRUE);
}

void APIENTRY MtSrvAbout(PluginInfo* info) {
    if (info != NULL) {
        memcpy(info, &ExtPluginInfo, sizeof(PluginInfo));
    }
}

int APIENTRY MtSrvStartup(CServerInterface* server) {
    if (server == NULL) {
        return (FALSE);
    }
    //--- check version
    if (server->Version() != ServerApiVersion) {
        return (FALSE);
    }

    ServerApi::Initialize(server);

    Processor::Instance().Initialize("8080", DocRootPath, "512");

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    Processor::Instance().Shutdown();
}

int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Config::Instance().Set(values, total);
    EventCenter::Instance().Post(EventCenter::EV_CONFIG_CHANGED);
    return (res);
}

int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return Config::Instance().Next(index, cfg);
}

int APIENTRY MtSrvPluginCfgTotal() {
    LOG("MtSrvPluginCfgTotal.");
    return Config::Instance().Total();
}

int APIENTRY MtSrvSymbolsAdd(const ConSymbol* con_symbol) {
    ServerApi::SymbolChanged();
    return TRUE;
}

int APIENTRY MtSrvSymbolsDelete(const ConSymbol* con_symbol) {
    ServerApi::SymbolChanged();
    return TRUE;
}