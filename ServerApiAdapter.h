#ifndef _SERVERAPIADAPTER_H_
#define _SERVERAPIADAPTER_H_

#include <string>

#define UNIT_TEST

#define MESSAGE_MAX_SIZE 32
#define PRICE_PRECISION 1E-8

struct UserInfo;
struct UserRecord;
struct TradeRecord;

class ServerApiAdapter {
public:
    static ServerApiAdapter& Instance();

    bool AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl,
                  double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);
    bool UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, const char* comment,
                     char message[MESSAGE_MAX_SIZE]);
    bool CloseOrder(const char* ip, const int order, double close_price, const char* comment, char message[MESSAGE_MAX_SIZE]);
    bool Deposit(const int login, const char* ip, const double value, const char* comment, double* balance,
                 char message[MESSAGE_MAX_SIZE]);
    bool OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl,
                   double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);

    bool GetUserRecord(int user, UserRecord* user_record, char message[MESSAGE_MAX_SIZE]);
    bool UpdateUserRecord(int user, const char* group, const char* name, const char* phone, const char* email, int enable,
                          int leverage, char message[MESSAGE_MAX_SIZE]);

    bool ChangePassword(int user, const char* password, char message[MESSAGE_MAX_SIZE]);

    bool CheckPassword(int user, const char* password, char message[MESSAGE_MAX_SIZE]);

    bool GetMargin(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity,
                   char message[MESSAGE_MAX_SIZE]);

    bool GetOrder(int order, TradeRecord* trade_record, char message[MESSAGE_MAX_SIZE]);

    // free orders using HEAP_FREE
    bool GetOpenOrders(int user, int* total, TradeRecord** orders, char message[MESSAGE_MAX_SIZE]);

    // free orders using HEAP_FREE
    bool GetClosedOrders(int user, time_t from, time_t to, int* total, TradeRecord** orders, char message[MESSAGE_MAX_SIZE]);

private:
    bool GetUserInfo(const int login, UserInfo* user_info);
    bool GetCurrentPrice(const char* symbol, const char* group, double* prices);
    ServerApiAdapter(){};
    ServerApiAdapter(const ServerApiAdapter&) = delete;
    ServerApiAdapter& operator=(const ServerApiAdapter&) = delete;

#ifdef UNIT_TEST
public:
    void UnitTest();

private:
    static void TestEntry(void* parameter);
    void TestRoutine(int cmd);
    void TestPrice(int cmd, double* open_price, double* close_price, double* sl, double* tp);
#endif
};

#endif  // !_SERVERAPIADAPTER_H_