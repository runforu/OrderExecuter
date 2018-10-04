#ifndef _SERVERAPIADAPTER_H_
#define _SERVERAPIADAPTER_H_

#include <process.h>
#include <windows.h>
#include "../include/MT4ServerAPI.h"
#include "Synchronizer.h"

#define UNIT_TEST

#define MESSAGE_MAX_SIZE 32
#define PRICE_PRECISION 1E-8

class ServerApiAdapter {
public:
    void Initialize();    void Shutdown();
    bool AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl,
                  double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);
    bool UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, const char* comment,
                     char message[MESSAGE_MAX_SIZE]);
    bool CloseOrder(const char* ip, const int order, double close_price, const char* comment, char message[MESSAGE_MAX_SIZE]);
    bool Deposit(const int login, const char* ip, const double value, const char* comment, char message[MESSAGE_MAX_SIZE]);
    bool OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl,
                   double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);

private:
    bool GetUserInfo(const int login, UserInfo* user_info);
    bool GetCurrentPrice(const char* symbol, const char* group, double* prices);
    
#ifdef UNIT_TEST
public:
    void UnitTest();
private:
    static void TestEntry(LPVOID parameter);
    void TestRoutine(int cmd);
    void TestPrice(int cmd, double* open_price, double* close_price, double* sl, double* tp);
#endif

private:
    //--- configurations
    Synchronizer m_synchronizer;
};

#endif  // !_SERVERAPIADAPTER_H_