#include "Config.h"
#include "Environment.h"
#include "EventCenter.h"
#include "LicenseService.h"
#include "Loger.h"
#include "Processor.h"
#include "ServerApi.h"
#include "../include/MT4ServerAPI.h"

extern const char *PLUGIN_VERSION_STRING;
#define PLUGIN_NAME "Order Executer"
#define PLUGIN_VERSION 2
#define PLUGIN_COPYRIGHT "DH Copyrigh."

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID /*lpReserved*/) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            char tmp[256], *cp;
            GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 5);
            //--- set root path
            COPY_STR(Environment::s_module_path, tmp);
            if ((cp = strrchr(Environment::s_module_path, '\\')) != NULL) {
                *(cp + 1) = 0;
                COPY_STR(Environment::s_doc_root, Environment::s_module_path);
                strcat(Environment::s_doc_root, "root\\");
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
        sprintf(info->name, "%s %s", PLUGIN_NAME, PLUGIN_VERSION_STRING);
        info->version = PLUGIN_VERSION;
        sprintf(info->copyright, "%s", PLUGIN_COPYRIGHT);
        memset(info->reserved, 0, sizeof(info->reserved));
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

    Processor::Instance().Initialize();

    Environment::Log();

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    Processor::Instance().Shutdown();
}

int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Config::Instance().Set(values, total);

#ifdef _LICENSE_VERIFICATION_
    LicenseService::Instance().ResetLicense();
#endif  // !_LICENSE_VERIFICATION_

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