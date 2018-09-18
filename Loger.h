#ifndef _LOGER_H_
#define _LOGER_H_

#include <windows.h>
#include "../include/MT4ServerAPI.h"
#include "common.h"

#ifdef _RELEASE_LOG_

#define _CODE_ 222
#define _IP_ "TestHelper"

class Loger {
public:
    static void out(const int code, LPCSTR ip, LPCSTR msg, ...);
    static void out(const int code, LPCSTR ip, const RequestInfo* request);
    static void out(const int code, LPCSTR ip, const TradeTransInfo* transaction);
    static void out(const int code, LPCSTR ip, const UserInfo* user_info);
    static void out(const int code, LPCSTR ip, const ConGroup* con_group);
    static void out(const int code, LPCSTR ip, const ConSymbol* con_symbol);
    static void out(const int code, LPCSTR ip, const TradeRecord* trade_record);
    static void out(const int code, LPCSTR ip, const TickAPI* tick);
    static const char* TradeTypeStr(int trade_type);
    static const char* TradeCmdStr(int trade_cmd);
    static const char* OrderTypeStr(int order_type);
};

#define TRADETYPE(trade_type) Loger::TradeTypeStr(trade_type)
#define TRADECMD(trade_cmd) Loger::TradeCmdStr(trade_cmd)
#define PRICEOPTION(price_option) Loger::PriceOptionStr(price_option)
#define ORDERTYPE(order_type) Loger::OrderTypeStr(order_type)

#define LOG(format, ...) Loger::out(_CODE_, _IP_, format, ##__VA_ARGS__);
#define LOG_INFO(info) 
//Loger::out(_CODE_, _IP_, info);

#else _RELEASE_LOG_

#define TRADETYPE(trade_type)
#define TRADECMD(trade_cmd)
#define PRICEOPTION(price_option)
#define ORDERTYPE(order_type)

#define LOG(format, ...)
#define LOG_INFO(inf)

#endif  //_RELEASE_LOG_

#endif  // !_LOGER_H_
