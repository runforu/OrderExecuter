#include "Processor.h"
#include "Config.h"
#include "Factory.h"
#include "Loger.h"


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
                *cp = 0;
                strcat(DocRootPath, "\\root\\");
            }
            //--- create configuration filename
            if ((cp = strrchr(tmp, '.')) != NULL) {
                *cp = 0;
                strcat(tmp, ".ini");
            }
            Factory::GetConfig()->Load(tmp);
            break;

        case DLL_THREAD_ATTACH:
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

    //--- save server interface link
    Factory::SetServerInterface(server);

    Factory::GetProcessor()->Initialize("8080", DocRootPath, "512");

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    Factory::GetProcessor()->Shutdown();
    Factory::SetServerInterface(NULL);
}
