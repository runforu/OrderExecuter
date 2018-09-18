#ifndef _AUTOTEST_H_
#define _AUTOTEST_H_

#include <process.h>
#include <windows.h>
#include "Synchronizer.h"

class Processor {
    friend class Factory;

private:
    //--- configurations
    volatile int m_disable_plugin;
    int m_interval;

    long m_shut_down_flag;

    Synchronizer m_synchronizer;

public:
    Processor();
    void Initialize();

    void AddOrder(const int login, const char* symbol, const int cmd, const char* ip);
    void UpdateOrder(const int login, const int ticket, const int cmd, const char* ip);
    void Shutdown();
    void SetBalance(const int login, const double value, const char* ip, const char* comment);
};

#endif  // !_AUTOTEST_H_