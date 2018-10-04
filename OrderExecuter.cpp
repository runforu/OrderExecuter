
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include "http_server/server.h"
#include "Config.h"
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"
#include "RequestHandlerProviderImp.h"

PluginInfo ExtPluginInfo = {"Order Executer", 1, "DH Copyrigh.", {0}};

int main(int argc, char* argv[]) {
    try {
        // Check command line arguments.
        if (argc != 5) {
            std::cerr << "Usage: HttpServer.exe <address> <port> <threads> <doc_root>\n";
            getchar();
            return 1;
        }

        // Initialise the server.
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);

        RequestHandlerProviderImp provider;
        http::server::server http_server(argv[1], argv[2], argv[4], num_threads, &provider);

        // Run the server until stopped.
        http_server.run();
    }
    catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    std::cerr << "http server stops.\n";
    getchar();
    return 0;
}


BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID /*lpReserved*/) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            char tmp[256], *cp;
            //--- create configuration filename
            GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 5);
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

    Factory::GetProcessor()->Initialize();

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    Factory::GetProcessor()->Shutdown();
    Factory::SetServerInterface(NULL);
}
