#ifndef _SERVERAPIADAPTER_H_
#define _SERVERAPIADAPTER_H_

#include <exception>
#include <string>
#include "Synchronizer.h"

constexpr auto MAX_SYMBOL_COUNT{256};
constexpr auto MAX_GROUP_COUNT{256};

#define PRICE_PRECISION 1E-8

struct UserInfo;
struct UserRecord;
struct TradeRecord;
struct CServerInterface;
struct ConSymbol;
struct ConGroup;
struct ErrorCode;

class ServerApi {
public:
    static void Initialize(CServerInterface* server_interface);
    static CServerInterface* Api();

    static bool DeleteUser(const int login, const ErrorCode** error_code);

    // change balance fast, delta is increase or decrease value
    static bool Withhold(const int login, const char* ip, const double delta, const char* comment, int* order, const ErrorCode** error_code);

    static bool BinaryOption(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double close_price, double profit,
                             double balance_change, const char* comment, const ErrorCode** error_code, int* order);

    static bool AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl, double tp, time_t expiration,
                         const char* comment, const ErrorCode** error_code, int* order);

    static bool UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, time_t expiration, const char* comment, const ErrorCode** error_code);

    static bool CloseOrder(const char* ip, const int order, double close_price, const char* comment, const ErrorCode** error_code);

    static bool Deposit(const int login, const char* ip, const double value, const char* comment, int* order, const ErrorCode** error_code);

    static bool OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price, double sl, double tp, time_t expiration,
                          const char* comment, const ErrorCode** error_code, int* order);

    static bool GetUserRecord(int user, UserRecord* user_record, const ErrorCode** error_code);

    static bool UpdateUserRecord(int user, const char* group, const char* name, const char* phone, const char* email, int enable, int leverage, const ErrorCode** error_code);

    static bool ChangePassword(int user, const char* password, const ErrorCode** error_code);

    static bool CheckPassword(int user, const char* password, const ErrorCode** error_code);

    static bool GetMargin(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity, const ErrorCode** error_code);

    static bool GetMarginInfo(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity, const ErrorCode** error_code);

    static bool GetOrder(int order, TradeRecord* trade_record, const ErrorCode** error_code);

    // free orders using HEAP_FREE
    static bool GetOpenOrders(int user, int* total, TradeRecord** orders, const ErrorCode** error_code);

    // free orders using HEAP_FREE
    static bool GetClosedOrders(int user, time_t from, time_t to, int* total, TradeRecord** orders, const ErrorCode** error_code);

    static bool IsOpening(const char* symbol, time_t time, bool* result, const ErrorCode** error_code);

    static bool CurrentTradeTime(time_t* time, const ErrorCode** error_code);

    static bool GetSymbolList(int* total, const ConSymbol** const symbols, const ErrorCode** error_code);

    static bool AddUser(int login, const char* name, const char* password, const char* group, const char* phone, const char* email, const char* lead_source, int leverage,
                        const ErrorCode** error_code, int* user);

    static void SymbolAdded(const ConSymbol* con_symbol);

    static void SymbolDeleted(const ConSymbol* con_symbol);

private:
    static bool GetUserInfo(const int login, UserInfo* user_info, const ErrorCode** error_code);

    static bool GetCurrentPrice(const char* symbol, const char* group, double* prices, const ErrorCode** error_code);

    static bool IsQuoteAlive(const char* symbol, const ErrorCode** error_code);

    ServerApi(){};
    ~ServerApi(){};
    ServerApi(const ServerApi&) = delete;
    ServerApi& operator=(const ServerApi&) = delete;

private:
    static CServerInterface* s_interface;
    static Synchronizer s_deposit_sync;
    static int s_symbol_count;
};

#endif  // !_SERVERAPIADAPTER_H_