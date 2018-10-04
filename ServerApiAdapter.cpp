#include <process.h>
#include <stdlib.h>
#include "Config.h"
#include "Factory.h"
#include "Loger.h"
#include "ServerApiAdapter.h"

void ServerApiAdapter::Shutdown() {
}

void ServerApiAdapter::Initialize() {}

void ServerApiAdapter::TestRoutine(int cmd) {
    FUNC_WARDER;

    int order = 0;
    const char* symbol = "USDJPY";
    const char* ip = "127.0.0.1";
    const char* group = "lmoa-main";
    const int login = 5;
    int volume = 100;
    long time_ms = 1000;
    char message[MESSAGE_MAX_SIZE];

    LOG("-----------------------------------------%s-----------------------", TRADECMD(cmd));
    double open_price, close_price, sl, tp;

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    OpenOrder(login, ip, symbol, cmd, volume, open_price, 0, 0, NULL, message, &order);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    UpdateOrder(ip, order, 0, sl, tp, NULL, message);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    UpdateOrder(ip, order, open_price, sl, tp, NULL, message);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    CloseOrder(ip, order, open_price, NULL, message);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    AddOrder(login, ip, symbol, cmd, volume, open_price, 0, 0, NULL, message, &order);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    CloseOrder(ip, order, close_price, NULL, message);
}

void ServerApiAdapter::TestPrice(int cmd, double* open_price, double* close_price, double* sl, double* tp) {
    double prices[2];
    double deviation = 0.043;
    GetCurrentPrice("USDJPY", "lmoa-main", prices);
    switch (cmd) {
        // market order sl, tp based on close price; pending order sl, tp based on open price
        case OP_BUY:
            *open_price = prices[1];
            *close_price = prices[0];
            *sl = *close_price - deviation;
            *tp = *close_price + deviation;
            break;
        case OP_SELL:
            *open_price = prices[0];
            *close_price = prices[1];
            *sl = *close_price + deviation;
            *tp = *close_price - deviation;
            break;
        case OP_BUY_LIMIT:
            // open price = ask - ConSymbol.stops_level
            *open_price = prices[1] - deviation;
            *close_price = prices[1];
            *sl = *open_price - deviation;
            *tp = *open_price + deviation;
            break;
        case OP_SELL_LIMIT:
            // open price = bid + ConSymbol.stops_level
            *open_price = prices[0] + deviation;
            *close_price = prices[1];
            *sl = *open_price + deviation;
            *tp = *open_price - deviation;
            break;
        case OP_BUY_STOP:
            // open price = ask + ConSymbol.stops_level
            *open_price = prices[1] + deviation;
            *close_price = prices[0];
            *sl = *open_price - deviation;
            *tp = *open_price + deviation;
            break;
        case OP_SELL_STOP:
            // open price = bid - ConSymbol.stops_level
            *open_price = prices[0] - deviation;
            *close_price = prices[1];
            *sl = *open_price + deviation;
            *tp = *open_price - deviation;
            break;
    }

    LOG("open_price = %f, close_price = %f, sl = %f, tp = %f,", *open_price, *close_price, *sl, *tp);
}

void ServerApiAdapter::TestEntry(LPVOID parameter) {
    for (int cmd = OP_BUY; cmd <= OP_SELL_STOP; cmd++) {
        ((ServerApiAdapter*)parameter)->TestRoutine(cmd);
    }
    _endthread();
}

void ServerApiAdapter::UnitTest() {
    _beginthread(TestEntry, 0, this);
}

bool ServerApiAdapter::OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                          double sl, double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order) {
    FUNC_WARDER;

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};

    //--- checks
    if (login <= 0 || cmd < OP_BUY || cmd > OP_SELL_STOP || symbol == NULL || volume <= 0) {
        LOG("OpenOrder: Invalid parameters");
        return false;
    }

    //--- get user info
    if (!GetUserInfo(login, &user_info)) {
        LOG("OpenOrder: UserInfoGet failed");
        return false;  // error
    }

    //--- get group config
    if (Factory::GetServerInterface()->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("OpenOrder: GroupsGet failed [%s]", user_info.group);
        return false;  // error
    }

    //--- get symbol config
    if (Factory::GetServerInterface()->SymbolsGet(symbol, &symbol_cfg) == FALSE) {
        LOG("OpenOrder: SymbolsGet failed [%s]", symbol);
        return false;  // error
    }

    if (Factory::GetServerInterface()->TradesCheckSessions(&symbol_cfg, Factory::GetServerInterface()->TradeTime()) == FALSE) {
        LOG("AddOrder: market closed", user_info.group);
        return false;  // error
    }

    //--- check long only
    if (symbol_cfg.long_only != FALSE && (cmd == OP_SELL || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP)) {
        LOG("OpenOrder: long only allowed");
        return false;  // long only
    }

    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("OpenOrder: close only allowed");
        return false;  // close only
    }

    //--- check secutiry
    if (Factory::GetServerInterface()->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("OpenOrder: trade disabled or market closed");
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- prepare transaction
    trade_trans_info.cmd = cmd;
    trade_trans_info.volume = volume;
    COPY_STR(trade_trans_info.symbol, symbol);
    //--- fill SL,TP, comment
    if (sl > PRICE_PRECISION) {
        trade_trans_info.sl = sl;
    }
    if (tp > PRICE_PRECISION) {
        trade_trans_info.tp = tp;
    }

    double prices[2] = {0};
    GetCurrentPrice(symbol, user_info.group, prices);
    if (open_price <= PRICE_PRECISION) {
        if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
            return false;
        }
        trade_trans_info.price = (cmd == OP_BUY) ? prices[1] : prices[0];
    } else {
        trade_trans_info.price = open_price;
    }

    COPY_STR(trade_trans_info.comment, comment);

    if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
        trade_trans_info.type = TT_ORDER_PENDING_OPEN;
    } else if (cmd == OP_BUY || cmd == OP_SELL) {
        trade_trans_info.type = TT_ORDER_MK_OPEN;
    }

    //--- check tick size
    if (Factory::GetServerInterface()->TradesCheckTickSize(open_price, &symbol_cfg) == FALSE) {
        LOG("OpenOrder: invalid price");
        return false;  // invalid price
    }

    //--- check volume
    if (Factory::GetServerInterface()->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("OpenOrder: invalid volume");
        return false;  // invalid volume
    }

    //--- check stops
    if (Factory::GetServerInterface()->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("OpenOrder: invalid stops");
        return false;  // invalid stops
    }

    //--- open order with margin check
    LOG_INFO(&trade_trans_info);
    if ((*order = Factory::GetServerInterface()->OrdersOpen(&trade_trans_info, &user_info)) == 0) {
        LOG("OpenOrder: OpenOrder failed");
        return false;  // error
    }

    //--- postion opened: return order
    return true;
}

bool ServerApiAdapter::AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                         double sl, double tp, const char* comment, char message[MESSAGE_MAX_SIZE], int* order) {
    FUNC_WARDER;

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};
    double profit = 0, margin = 0, free_margin = 0, prev_margin = 0;

    //--- checks
    if (login <= 0 || cmd < OP_BUY || cmd > OP_SELL_STOP || symbol == NULL || volume <= 0) {
        LOG("AddOrder: Invalid parameters");
        return false;
    }

    //--- get user info
    if (!GetUserInfo(login, &user_info)) {
        LOG("AddOrder: GetUserInfo failed");
        strncpy(message, "AddOrder: GetUserInfo failed", MESSAGE_MAX_SIZE);
        return false;  // error
    }

    //--- get group config
    if (Factory::GetServerInterface()->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("AddOrder: GroupsGet failed [%s]", user_info.group);
        return false;  // error
    }

    //--- get symbol config
    if (Factory::GetServerInterface()->SymbolsGet(symbol, &symbol_cfg) == FALSE) {
        LOG("AddOrder: SymbolsGet failed [%s]", symbol);
        return false;  // error
    }

    if (Factory::GetServerInterface()->TradesCheckSessions(&symbol_cfg, Factory::GetServerInterface()->TradeTime()) == FALSE) {
        LOG("AddOrder: market closed", user_info.group);
        return false;  // error
    }

    //--- check long only
    if (symbol_cfg.long_only != FALSE && (cmd == OP_SELL || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP)) {
        LOG("AddOrder: long only allowed");
        return false;  // long only
    }
    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("AddOrder: close only allowed");
        return false;  // close only
    }

    //--- check secutiry
    if (Factory::GetServerInterface()->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("AddOrder: trade disabled or market closed");
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- prepare transaction
    trade_trans_info.cmd = cmd;
    trade_trans_info.volume = volume;
    COPY_STR(trade_trans_info.symbol, symbol);

    //--- fill SL,TP, comment
    if (sl > PRICE_PRECISION) {
        trade_trans_info.sl = sl;
    }
    if (tp > PRICE_PRECISION) {
        trade_trans_info.tp = tp;
    }

    double prices[2] = {0};
    GetCurrentPrice(symbol, user_info.group, prices);
    if (open_price <= PRICE_PRECISION) {
        if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
            return false;
        }
        trade_trans_info.price = (cmd == OP_BUY) ? prices[1] : prices[0];
    } else {
        trade_trans_info.price = open_price;
    }

    if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
        trade_trans_info.type = TT_ORDER_PENDING_OPEN;
    } else if (cmd == OP_BUY || cmd == OP_SELL) {
        trade_trans_info.type = TT_ORDER_MK_OPEN;
    }

    //--- check tick size
    if (Factory::GetServerInterface()->TradesCheckTickSize(trade_record.open_price, &symbol_cfg) == FALSE) {
        LOG("AddOrder: invalid price");
        return false;  // invalid price
    }

    //--- check volume
    if (Factory::GetServerInterface()->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("AddOrder: invalid volume");
        return false;  // invalid volume
    }

    //--- check margin
    if (cmd == OP_BUY || cmd == OP_SELL) {
        margin = Factory::GetServerInterface()->TradesMarginCheck(&user_info, &trade_trans_info, &profit, &free_margin,
                                                                  &prev_margin);
        if ((free_margin + group_cfg.credit) < 0 && (symbol_cfg.margin_hedged_strong != FALSE || prev_margin <= margin)) {
            LOG("AddOrder: not enough margin");
            return false;  // no enough margin
        }
    }

    //--- prepare new trade state of order
    trade_record.login = login;
    trade_record.cmd = cmd;
    trade_record.open_price = trade_trans_info.price;
    trade_record.volume = volume;
    trade_record.close_price = ((cmd == OP_BUY || cmd == OP_BUY_LIMIT || cmd == OP_BUY_STOP) ? prices[0] : prices[1]);
    trade_record.open_time = Factory::GetServerInterface()->TradeTime();
    trade_record.digits = symbol_cfg.digits;

    COPY_STR(trade_record.symbol, symbol);
    COPY_STR(trade_record.comment, comment);

    trade_record.sl = trade_trans_info.sl;
    trade_record.tp = trade_trans_info.tp;

    //--- check stops
    if (Factory::GetServerInterface()->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("AddOrder: invalid stops");
        return false;  // invalid stops
    }

    //--- add order into database directly
    LOG_INFO(&trade_record);
    if ((*order = Factory::GetServerInterface()->OrdersAdd(&trade_record, &user_info, &symbol_cfg)) == 0) {
        LOG("AddOrder: OrdersAdd failed");
        return false;  // error
    }

    //--- postion opened: return order
    return true;
}

bool ServerApiAdapter::UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, const char* comment,
                            char message[MESSAGE_MAX_SIZE]) {
    FUNC_WARDER;

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};

    if (order <= 0) {
        LOG("AddOrder: invalid order");
        return false;
    }

    //--- get order
    if (Factory::GetServerInterface()->OrdersGet(order, &trade_record) == FALSE) {
        LOG("UpdateOrder: OrdersGet failed");
        return false;  // error
    }

    //--- check state
    if (trade_record.cmd < OP_BUY || trade_record.cmd > OP_SELL_STOP) {
        LOG("UpdateOrder: invalid order command");
        return false;  // order already activated
    }

    //--- get user info
    if (!GetUserInfo(trade_record.login, &user_info)) {
        LOG("UpdateOrder: GetUserInfo failed");
        return false;  // error
    }

    //--- get group config
    if (Factory::GetServerInterface()->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("UpdateOrder: GroupsGet failed [%s]", user_info.group);
        return false;  // error
    }

    //--- get symbol config
    if (Factory::GetServerInterface()->SymbolsGet(trade_record.symbol, &symbol_cfg) == FALSE) {
        LOG("UpdateOrder: SymbolsGet failed [%s]", trade_record.symbol);
        return false;  // error
    }

    if (Factory::GetServerInterface()->TradesCheckSessions(&symbol_cfg, Factory::GetServerInterface()->TradeTime()) == FALSE) {
        LOG("UpdateOrder: market closed", user_info.group);
        return false;  // error
    }

    //--- check tick size
    if (Factory::GetServerInterface()->TradesCheckTickSize(open_price, &symbol_cfg) == FALSE) {
        LOG("UpdateOrder: invalid price");
        return false;  // invalid price
    }

    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("UpdateOrder: close only");
        return false;
    }

    if (sl > PRICE_PRECISION) {
        trade_trans_info.sl = sl;
        trade_record.sl = sl;
        LOG("UpdateOrder: sl is set to %f", sl);
    }

    if (tp > PRICE_PRECISION) {
        trade_trans_info.tp = tp;
        trade_record.tp = tp;
        LOG("UpdateOrder: tp is set to %f", tp);
    }

    if (open_price > PRICE_PRECISION) {
        // modification of open price is forbidden
        if (trade_record.cmd == OP_BUY || trade_record.cmd == OP_SELL) {
            LOG("UpdateOrder: modification of open price [%f] is forbidden", open_price);
            return false;
        }
        trade_record.open_price = open_price;
        trade_trans_info.price = open_price;
        LOG("UpdateOrder: open price is set to [%f]", open_price);
    } else {
        // sl tp need the open price to pass TradesCheckStops
        trade_trans_info.price = trade_record.open_price;
    }

    trade_trans_info.cmd = trade_record.cmd;
    if (trade_record.cmd >= OP_BUY_LIMIT && trade_record.cmd <= OP_SELL_STOP) {
        trade_trans_info.type = TT_ORDER_PENDING_OPEN;
    } else if (trade_record.cmd == OP_BUY || trade_record.cmd == OP_SELL) {
        trade_trans_info.type = TT_ORDER_MK_OPEN;
    }

    //--- check secutiry
    if (Factory::GetServerInterface()->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("UpdateOrder: trade disabled or market closed");
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- check stops
    LOG_INFO(&trade_trans_info);
    if (Factory::GetServerInterface()->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("UpdateOrder: invalid stops");
        return false;  // invalid stops
    }

    COPY_STR(trade_record.comment, comment);
    if (Factory::GetServerInterface()->OrdersUpdate(&trade_record, &user_info, UPDATE_NORMAL) == FALSE) {
        LOG("UpdateOrder: OrdersUpdate failed");
        return false;  // error
    }

    return true;
}

bool ServerApiAdapter::CloseOrder(const char* ip, const int order, double close_price, const char* comment,
                           char message[MESSAGE_MAX_SIZE]) {
    FUNC_WARDER;

    UserInfo user_info = {0};
    ConGroup group_cfg = {0};
    ConSymbol symbol_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};

    if (order <= 0) {
        LOG("CloseOrder: Invalid order");
        return false;
    }

    //--- get order
    if (Factory::GetServerInterface()->OrdersGet(order, &trade_record) == FALSE) {
        LOG("CloseOrder: OrdersGet failed");
        return false;  // error
    }

    //--- get user info
    if (!GetUserInfo(trade_record.login, &user_info)) {
        LOG("CloseOrder: GetUserInfo failed");
        return false;  // error
    }

    LOG_INFO(&trade_record);

    //--- get group config
    if (Factory::GetServerInterface()->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("CloseOrder: GroupsGet failed [%s]", user_info.group);
        return false;  // error
    }

    //--- get symbol config
    if (Factory::GetServerInterface()->SymbolsGet(trade_record.symbol, &symbol_cfg) == FALSE) {
        LOG("CloseOrder: SymbolsGet failed [%s]", trade_record.symbol);
        return false;  // error
    }

    if (Factory::GetServerInterface()->TradesCheckSessions(&symbol_cfg, Factory::GetServerInterface()->TradeTime()) == FALSE) {
        LOG("CloseOrder: market closed", user_info.group);
        return false;  // error
    }

    //--- prepare transaction
    trade_trans_info.order = order;
    trade_trans_info.price = close_price;
    trade_trans_info.volume = trade_record.volume;
    trade_trans_info.cmd = trade_record.cmd;
    COPY_STR(trade_trans_info.comment, comment);
    COPY_STR(trade_trans_info.symbol, trade_record.symbol);

    if (trade_record.cmd >= OP_BUY_LIMIT && trade_record.cmd <= OP_SELL_STOP) {
        trade_trans_info.type = TT_ORDER_DELETE;
    } else {
        trade_trans_info.type = TT_ORDER_MK_CLOSE;
    }

    //--- check tick size
    if (Factory::GetServerInterface()->TradesCheckTickSize(close_price, &symbol_cfg) == FALSE) {
        LOG("CloseOrder: invalid price");
        return false;  // invalid price
    }

    //--- check secutiry
    if (Factory::GetServerInterface()->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("OrdersUpdateClose: trade disabled or market closed");
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- check volume
    if (Factory::GetServerInterface()->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("OrdersUpdateClose: invalid volume");
        return false;  // invalid volume
    }

    //--- check stops
    if (Factory::GetServerInterface()->TradesCheckFreezed(&symbol_cfg, &group_cfg, &trade_record) != RET_OK) {
        LOG("OrdersUpdateClose: position freezed");
        return false;  // position freezed
    }

    //--- close position
    LOG_INFO(&trade_trans_info);
    trade_trans_info.price = close_price;
    if (Factory::GetServerInterface()->OrdersClose(&trade_trans_info, &user_info) == FALSE) {
        LOG("CloseOrder: CloseOrder failed");
        return false;  // error
    }

    //--- position closed
    return true;
}

bool ServerApiAdapter::Deposit(const int login, const char* ip, const double value, const char* comment,
                        char message[MESSAGE_MAX_SIZE]) {
    FUNC_WARDER;

    UserInfo user = {0};
    if (!GetUserInfo(login, &user)) {
        return false;
    }
    if (Factory::GetServerInterface()->ClientsChangeBalance(login, &user.grp, user.balance + value, comment) == 0) {
        return false;
    }
    return true;
}

bool ServerApiAdapter::GetUserInfo(const int login, UserInfo* user_info) {
    UserRecord user_record = {0};

    if (Factory::GetServerInterface()->ClientsUserInfo(login, &user_record) == FALSE) {
        return false;
    }

    user_info->balance = user_record.balance;
    user_info->login = user_record.login;
    user_info->credit = user_record.credit;
    user_info->enable = user_record.enable;
    user_info->leverage = user_record.leverage;
    COPY_STR(user_info->group, user_record.group);
    COPY_STR(user_info->name, user_record.name);

    ConGroup grpcfg = {0};
    if (Factory::GetServerInterface()->GroupsGet(user_info->group, &grpcfg) == FALSE) {
        return false;
    }
    user_info->grp = grpcfg;

    return true;
}

bool ServerApiAdapter::GetCurrentPrice(const char* symbol, const char* group, double* prices) {
    ConGroup grpcfg = {0};
    if (Factory::GetServerInterface()->GroupsGet(group, &grpcfg) == FALSE) {
        return false;
    }
    if (Factory::GetServerInterface()->HistoryPricesGroup(symbol, &grpcfg, prices) != RET_OK) {
        return false;
    }
    return true;
}
