#ifndef _AUTOTEST_H_
#define _AUTOTEST_H_

#include <process.h>
#include <windows.h>
#include "../include/MT4ServerAPI.h"
#include "Synchronizer.h"

#define MESSAGE_MAX_SIZE 32
#define PRICE_PRECISION 1E-8

class Processor {
    friend class Factory;

public:
    Processor();
    void Initialize();

    void UnitTest();

    bool AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl,
                  double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);
    bool UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, const char* comment,
                     char message[MESSAGE_MAX_SIZE]);
    bool CloseOrder(const char* ip, const int order, double close_price, const char* comment, char message[MESSAGE_MAX_SIZE]);

    void Shutdown();
    bool SetBalance(const int login, const char* ip, const double value, const char* comment, char message[MESSAGE_MAX_SIZE]);
    bool OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl,
                   double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);

private:
    bool GetUserInfo(const int login, UserInfo* user_info);
    bool GetCurrentPrice(const char* symbol, const char* group, double* prices);

    static void TestEntry(LPVOID parameter);
    void TestRoutine(int cmd);
    void TestPrice(int cmd, double* open_price, double* close_price, double* sl, double* tp);

private:
    //--- configurations
    long m_shut_down_flag;
    Synchronizer m_synchronizer;
};

#endif  // !_AUTOTEST_H_