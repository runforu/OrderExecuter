#ifndef _SERVERAPIADAPTER_H_
#define _SERVERAPIADAPTER_H_

#include <exception>
#include <string>

#define UNIT_TEST

#define MESSAGE_MAX_SIZE 32
#define PRICE_PRECISION 1E-8

struct UserInfo;
struct UserRecord;
struct TradeRecord;
struct CServerInterface;

class ServerApi {
public:
    static void Initialize(CServerInterface* server_interface);
    static CServerInterface* Api();

    static bool AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                         double sl, double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);
    static bool UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, const char* comment,
                            char message[MESSAGE_MAX_SIZE]);
    static bool CloseOrder(const char* ip, const int order, double close_price, const char* comment,
                           char message[MESSAGE_MAX_SIZE]);
    static bool Deposit(const int login, const char* ip, const double value, const char* comment, double* balance,
                        char message[MESSAGE_MAX_SIZE]);
    static bool OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                          double sl, double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order);

    static bool GetUserRecord(int user, UserRecord* user_record, char message[MESSAGE_MAX_SIZE]);
    static bool UpdateUserRecord(int user, const char* group, const char* name, const char* phone, const char* email,
                                 int enable, int leverage, char message[MESSAGE_MAX_SIZE]);

    static bool ChangePassword(int user, const char* password, char message[MESSAGE_MAX_SIZE]);

    static bool CheckPassword(int user, const char* password, char message[MESSAGE_MAX_SIZE]);

    static bool GetMargin(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity,
                          char message[MESSAGE_MAX_SIZE]);

    static bool GetOrder(int order, TradeRecord* trade_record, char message[MESSAGE_MAX_SIZE]);

    // free orders using HEAP_FREE
    static bool GetOpenOrders(int user, int* total, TradeRecord** orders, char message[MESSAGE_MAX_SIZE]);

    // free orders using HEAP_FREE
    static bool GetClosedOrders(int user, time_t from, time_t to, int* total, TradeRecord** orders,
                                char message[MESSAGE_MAX_SIZE]);

private:
    static bool GetUserInfo(const int login, UserInfo* user_info);
    static bool GetCurrentPrice(const char* symbol, const char* group, double* prices);

    ServerApi(){};
    ~ServerApi(){};
    ServerApi(const ServerApi&) = delete;
    ServerApi& operator=(const ServerApi&) = delete;

private:
    static CServerInterface* s_interface;

#ifdef UNIT_TEST
public:
    static void UnitTest();

private:
    static void TestEntry(void* parameter);
    static void TestRoutine(int cmd);
    static void TestPrice(int cmd, double* open_price, double* close_price, double* sl, double* tp);
#endif
};

#endif  // !_SERVERAPIADAPTER_H_