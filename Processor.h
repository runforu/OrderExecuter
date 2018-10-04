#ifndef _AUTOTEST_H_
#define _AUTOTEST_H_

#include <process.h>
#include <windows.h>
#include "../include/MT4ServerAPI.h"
#include "Synchronizer.h"
#include "ServerApiAdapter.h"

class Processor {
    friend class Factory;

public:
    void Initialize();
    void Shutdown();

private:
    ServerApiAdapter m_api;
    Synchronizer m_synchronizer;
};

#endif  // !_AUTOTEST_H_