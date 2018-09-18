#include <process.h>
#include <stdlib.h>
#include "Processor.h"
#include "Config.h"
#include "Factory.h"
#include "Loger.h"

Processor::Processor() : m_disable_plugin(0), m_shut_down_flag(0), m_interval(1000) {}

void Processor::Shutdown() {
    InterlockedExchange(&m_shut_down_flag, 1);
}

void Processor::Initialize() {
}

void Processor::AddOrder(const int login, const char* symbol, const int cmd, const char* ip) {
    LOG("--->AddOrder");

    UserRecord user = {0};
    Factory::GetServerInterface()->ClientsUserInfo(login, &user);
    UserInfo user_info = {0};
    user_info.balance = user.balance;
    user_info.login = user.login;
    user_info.credit = user.credit;
    user_info.enable = user.enable;
    user_info.leverage = user.leverage;
    COPY_STR(user_info.group, user.group);
    COPY_STR(user_info.name, user.name);
    COPY_STR(user_info.ip, "order robot");

    ConGroup grpcfg = {0};
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);
    user_info.grp = grpcfg;

    ConSymbol symcfg = {0};
    srand(GetTickCount());
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);

    TradeRecord trade = {0};
    trade.login = 5;
    trade.volume = 100;
    trade.open_time = Factory::GetServerInterface()->TradeTime();
    trade.digits = symcfg.digits;
    COPY_STR(trade.comment, "robot placed");
    COPY_STR(trade.symbol, symbol);
    trade.cmd = rand() % 4 + 2;

    double prices[2] = {0};
    Factory::GetServerInterface()->HistoryPricesGroup(symbol, &grpcfg, prices);
    double tmp = NormalizeDouble(3.0 / pow(10, symcfg.digits), symcfg.digits);

    trade.open_price =
        (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (prices[0] - tmp) : (prices[1] + tmp);

    LOG("prices [%f, %f] %f %d %d", prices[0], prices[1], tmp, symcfg.digits, symcfg.digits ^ 10);
    trade.close_price = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP ? prices[0] : prices[1]);
    trade.tp = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (trade.open_price + tmp)
                                                                                              : (trade.open_price - tmp);
    trade.sl = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (trade.open_price - tmp)
                                                                                              : (trade.open_price + tmp);
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);
    int order_id = Factory::GetServerInterface()->OrdersAdd(&trade, &user_info, &symcfg);
    LOG("order %d added", order_id);
    LOG_INFO(&trade);

    LOG("<---AddOrder");
}

void Processor::UpdateOrder(const int login, const int order, const int cmd, const char* ip) {
    LOG("--->UpdateOrder");
    if (m_shut_down_flag) {
        return;
    }

    if (order <= 0) {
        return;
    }

    UserRecord user_record = { 0 };
    Factory::GetServerInterface()->ClientsUserInfo(login, &user_record);
    UserInfo user_info = { 0 };
    user_info.balance = user_record.balance;
    user_info.login = user_record.login;
    user_info.credit = user_record.credit;
    user_info.enable = user_record.enable;
    user_info.leverage = user_record.leverage;
    COPY_STR(user_info.group, user_record.group);
    COPY_STR(user_info.name, user_record.name);
    COPY_STR(user_info.ip, "");
    ConGroup grpcfg = { 0 };
    Factory::GetServerInterface()->GroupsGet(user_record.group, &grpcfg);
    user_info.grp = grpcfg;

    LOG_INFO(&user_info);

    TradeRecord trade_record = { 0 };
    Factory::GetServerInterface()->OrdersGet(order, &trade_record);
    trade_record.close_time = Factory::GetServerInterface()->TradeTime();
    double prices[2] = { 0 };
    Factory::GetServerInterface()->HistoryPricesGroup(trade_record.symbol, &grpcfg, prices);
    trade_record.close_price = prices[0];
    COPY_STR(trade_record.comment, "Closed by executer ^_^");
    Factory::GetServerInterface()->OrdersUpdate(&trade_record, &user_info, UPDATE_CLOSE);
    LOG("<---UpdateOrder");
}

void Processor::SetBalance(const int login, const double value, const char* ip, const char* comment) {
    LOG("--->SetBalance");
    UserRecord user = {0};
    Factory::GetServerInterface()->ClientsUserInfo(5, &user);
    ConGroup grpcfg = {0};
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);
    Factory::GetServerInterface()->ClientsChangeBalance(5, &grpcfg, value, comment);
    LOG("--->SetBalance");
}

