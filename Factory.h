#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <windows.h>
#include "Config.h"
#include "Synchronizer.h"
#include "Processor.h"

class Factory {
private:
    Config m_config;
    Processor m_auto_test;
    CServerInterface* m_server = NULL;

private:
    static Factory m_instance;

public:
    inline static Config* GetConfig() { return &m_instance.m_config; }
    inline static void SetServerInterface(CServerInterface* server) { m_instance.m_server = server; }
    inline static Processor* GetProcessor() { return &m_instance.m_auto_test; }
    inline static CServerInterface* GetServerInterface() { return m_instance.m_server; }

private:
    Factory() : m_server(NULL) {}
    Factory(Factory const&) {}
    void operator=(Factory const&) {}
    ~Factory() {}
};

#endif  //--- _FACTORY_H_