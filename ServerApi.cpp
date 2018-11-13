#include <process.h>
#include <stdlib.h>
#include "Config.h"
#include "Loger.h"
#include "ServerApi.h"
#include "common.h"
#include "../include/MT4ServerAPI.h"

#define MESSAGE_COPY(message, src)                       \
    {                                                    \
        if (message != NULL && src != NULL) {            \
            strncpy(message, src, MESSAGE_MAX_SIZE - 1); \
            message[MESSAGE_MAX_SIZE - 1] = 0;           \
        }                                                \
    }

const ErrorCode ServerApi::EC_OK = {0, "SUCCESS"};
const ErrorCode ServerApi::EC_UNKNOWN_ERROR = {-1, "Unkown error"};
const ErrorCode ServerApi::EC_BAD_PARAMETER = {-2, "Bad parameters"};
const ErrorCode ServerApi::EC_BAD_TIME_SPAN = {-3, "Bad time span"};
const ErrorCode ServerApi::EC_INVALID_USER_ID = {-4, "Invalid user id"};
const ErrorCode ServerApi::EC_INVALID_SERVER_INTERFACE = {-5, "No server interface"};
const ErrorCode ServerApi::EC_GROUP_NOT_FOUND = {-6, "User group not found"};
const ErrorCode ServerApi::EC_GET_PRICE_ERROR = {-7, "Fail to gurrent price"};
const ErrorCode ServerApi::EC_SYMBOL_NOT_FOUND = {-8, "Symbol not found"};
const ErrorCode ServerApi::EC_INVALID_ORDER_TICKET = {-9, "Invalid order ticket"};
const ErrorCode ServerApi::EC_CHANGE_OPEN_PRICE = {-10, "Open price not allowed to modify for open order"};
const ErrorCode ServerApi::EC_CLOSE_ONLY = {-11, "Update order is not allowed [close only]"};
const ErrorCode ServerApi::EC_WRONG_PASSWORD = {-12, "Password at lest 6 characters"};
const ErrorCode ServerApi::EC_PENDING_ORDER_WITHOUT_OPEN_PRICE = {-13, "Pending order without open price"};

const ErrorCode ServerApi::EC_NO_CONNECT = {6, "No connection"};
const ErrorCode ServerApi::EC_ACCOUNT_DISABLED = {64, "Account blocked"};
const ErrorCode ServerApi::EC_BAD_ACCOUNT_INFO = {65, "Bad account info"};
const ErrorCode ServerApi::EC_TRADE_TIMEOUT = {128, "Trade transatcion timeou expired"};
const ErrorCode ServerApi::EC_TRADE_BAD_PRICES = {129, "Order has wrong prices"};
const ErrorCode ServerApi::EC_TRADE_BAD_STOPS = {130, "Wrong stops level"};
const ErrorCode ServerApi::EC_TRADE_BAD_VOLUME = {131, "Wrong lot size"};
const ErrorCode ServerApi::EC_TRADE_MARKET_CLOSED = {132, "Market closed"};
const ErrorCode ServerApi::EC_TRADE_DISABLE = {133, "Trade disabled"};
const ErrorCode ServerApi::EC_TRADE_NO_MONEY = {134, "No enough money for order execution"};
const ErrorCode ServerApi::EC_TRADE_PRICE_CHANGED = {135, "Price changed"};
const ErrorCode ServerApi::EC_TRADE_OFFQUOTES = {136, "No quotes"};
const ErrorCode ServerApi::EC_TRADE_BROKER_BUSY = {137, "Broker is busy"};
const ErrorCode ServerApi::EC_TRADE_ORDER_LOCKED = {138, "Order is proceed by dealer and cannot be changed"};
const ErrorCode ServerApi::EC_TRADE_LONG_ONLY = {139, "Allowed only BUY orders"};
const ErrorCode ServerApi::EC_TRADE_TOO_MANY_REQ = {140, "Too many requests from one client"};
const ErrorCode ServerApi::EC_TRADE_MODIFY_DENIED = {144, "Modification denied because order too close to market"};

CServerInterface* ServerApi::s_interface = NULL;

void ServerApi::Initialize(CServerInterface* server_interface) {
    s_interface = server_interface;
}

CServerInterface* ServerApi::Api() throw(...) {
    return s_interface;
}

#ifdef UNIT_TEST

void ServerApi::UnitTest() {
    _beginthread(TestEntry, 0, 0);
}

void ServerApi::TestEntry(void* parameter) {
    for (int cmd = OP_BUY; cmd <= OP_SELL_STOP; cmd++) {
        TestRoutine(cmd);
    }
    _endthread();
}

void ServerApi::TestRoutine(int cmd) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        return;
    }

    int order = 0;
    const char* symbol = "USDJPY";
    const char* ip = "127.0.0.1";
    const char* group = "lmoa-main";
    const int login = 5;
    int volume = 100;
    long time_ms = 1000;
    const ErrorCode** error_code;

    LOG("-----------------------------------------%s-----------------------", TradeCmdStr(cmd));
    double open_price, close_price, sl, tp;

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    OpenOrder(login, ip, symbol, cmd, volume, open_price, 0, 0, NULL, error_code, &order);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    UpdateOrder(ip, order, 0, sl, tp, NULL, error_code);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    UpdateOrder(ip, order, open_price, sl, tp, NULL, error_code);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    CloseOrder(ip, order, open_price, NULL, error_code);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    AddOrder(login, ip, symbol, cmd, volume, open_price, 0, 0, NULL, error_code, &order);

    Sleep(time_ms);
    TestPrice(cmd, &open_price, &close_price, &sl, &tp);
    CloseOrder(ip, order, close_price, NULL, error_code);
}

void ServerApi::TestPrice(int cmd, double* open_price, double* close_price, double* sl, double* tp) {
    if (s_interface == NULL) {
        return;
    }

    double prices[2];
    double deviation = 0.043;
    const ErrorCode* error_code;
    GetCurrentPrice("USDJPY", "lmoa-main", prices, &error_code);
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

#endif

bool ServerApi::OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                          double sl, double tp, const char* comment, const ErrorCode** error_code, int* order) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};

    //--- checks
    if (login <= 0 || cmd < OP_BUY || cmd > OP_SELL_STOP || symbol == NULL || volume <= 0) {
        LOG("OpenOrder: Invalid parameters");
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    //--- get user info
    if (!GetUserInfo(login, &user_info, error_code)) {
        LOG("OpenOrder: UserInfoGet failed");
        return false;  // error
    }

    //--- get group config
    if (s_interface->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("OpenOrder: GroupsGet failed [%s]", user_info.group);
        *error_code = &EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(symbol, &symbol_cfg) == FALSE) {
        LOG("OpenOrder: SymbolsGet failed [%s]", symbol);
        *error_code = &EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("AddOrder: market closed", user_info.group);
        *error_code = &EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- check long only
    if (symbol_cfg.long_only != FALSE && (cmd == OP_SELL || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP)) {
        LOG("OpenOrder: long only allowed");
        *error_code = &EC_TRADE_LONG_ONLY;
        return false;  // long only
    }

    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("OpenOrder: close only allowed");
        *error_code = &EC_CLOSE_ONLY;
        return false;  // close only
    }

    //--- check secutiry
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("OpenOrder: trade disabled or market closed");
        if (rt == RET_ERROR) {
            *error_code = &EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &EC_TRADE_OFFQUOTES;
        }
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
    GetCurrentPrice(symbol, user_info.group, prices, error_code);
    if (open_price <= PRICE_PRECISION) {
        if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
            *error_code = &EC_PENDING_ORDER_WITHOUT_OPEN_PRICE;
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
    if (s_interface->TradesCheckTickSize(open_price, &symbol_cfg) == FALSE) {
        LOG("OpenOrder: invalid price");
        *error_code = &EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check volume
    if (s_interface->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("OpenOrder: invalid volume");
        *error_code = &EC_TRADE_BAD_VOLUME;
        return false;  // invalid volume
    }

    //--- check stops
    if (s_interface->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("OpenOrder: invalid stops");
        *error_code = &EC_TRADE_BAD_STOPS;
        return false;  // invalid stops
    }

    //--- open order with margin check
    LOG_INFO(&trade_trans_info);
    if ((*order = s_interface->OrdersOpen(&trade_trans_info, &user_info)) == 0) {
        LOG("OpenOrder: OpenOrder failed");
        *error_code = &EC_UNKNOWN_ERROR;
        return false;  // error
    }

    //--- postion opened: return order
    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetUserRecord(int user, UserRecord* user_record, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user_record == NULL || user < 0) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->ClientsUserInfo(user, user_record) == FALSE) {
        *error_code = &EC_INVALID_USER_ID;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::UpdateUserRecord(int user, const char* group, const char* name, const char* phone, const char* email,
                                 int enable, int leverage, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    UserRecord user_record = {0};

    if (s_interface->ClientsUserInfo(user, &user_record) == FALSE) {
        *error_code = &EC_INVALID_USER_ID;
        return false;
    }

    if (group != NULL && strnlen_s(group, sizeof(user_record.group)) != 0) {
        COPY_STR(user_record.group, group);
    }
    if (group != NULL && strnlen_s(name, sizeof(user_record.name)) != 0) {
        COPY_STR(user_record.name, name);
    }
    if (group != NULL && strnlen_s(phone, sizeof(user_record.phone)) != 0) {
        COPY_STR(user_record.phone, phone);
    }
    if (group != NULL && strnlen_s(email, sizeof(user_record.email)) != 0) {
        COPY_STR(user_record.email, email);
    }
    if (enable != -1) {
        user_record.enable = enable;
    }
    if (leverage != -1) {
        user_record.leverage = leverage;
    }
    if (s_interface->ClientsUserUpdate(&user_record) == FALSE) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::ChangePassword(int user, const char* password, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    if (strnlen_s(password, 32) < 5) {
        *error_code = &EC_WRONG_PASSWORD;
        return false;
    }

    if (s_interface->ClientsChangePass(user, password, FALSE, FALSE) == FALSE) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::CheckPassword(int user, const char* password, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->ClientsCheckPass(user, password, FALSE) == FALSE) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetMargin(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity,
                          const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0 || user_info == NULL || margin == NULL || freemargin == NULL || equity == NULL) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->TradesMarginGet(user, user_info, margin, freemargin, equity) == FALSE) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetOrder(int order, TradeRecord* trade_record, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (order < 0 || trade_record == NULL) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->OrdersGet(order, trade_record) == FALSE) {
        *error_code = &EC_INVALID_ORDER_TICKET;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetOpenOrders(int user, int* total, TradeRecord** orders, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    UserInfo user_info = {0};
    if (GetUserInfo(user, &user_info, error_code) == FALSE) {
        return false;
    }

    if ((*orders = s_interface->OrdersGetOpen(&user_info, total)) == FALSE) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetClosedOrders(int user, time_t from, time_t to, int* total, TradeRecord** orders,
                                const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0 || error_code == NULL) {
        return false;
    }
    if (from == -1) {
        from = s_interface->TradeTime() - 7 * 24 * 60 * 60;
    }
    if (to == -1) {
        to = s_interface->TradeTime();
    }

    if (to - from > 7 * 24 * 60 * 60) {
        *error_code = &EC_BAD_TIME_SPAN;
        return false;
    }

    int users[1] = {user};
    if ((*orders = s_interface->OrdersGetClosed(from, to, users, 1, total)) == FALSE) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                         double sl, double tp, const char* comment, const ErrorCode** error_code, int* order) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};
    double profit = 0, margin = 0, free_margin = 0, prev_margin = 0;

    //--- checks
    if (login <= 0 || cmd < OP_BUY || cmd > OP_SELL_STOP || symbol == NULL || volume <= 0) {
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    //--- get user info
    if (!GetUserInfo(login, &user_info, error_code)) {
        LOG("AddOrder: GetUserInfo failed");
        return false;  // error
    }

    //--- get group config
    if (s_interface->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("AddOrder: GroupsGet failed [%s]", user_info.group);
        *error_code = &EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(symbol, &symbol_cfg) == FALSE) {
        LOG("AddOrder: SymbolsGet failed [%s]", symbol);
        *error_code = &EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("AddOrder: market closed", user_info.group);
        *error_code = &EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- check long only
    if (symbol_cfg.long_only != FALSE && (cmd == OP_SELL || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP)) {
        LOG("AddOrder: long only allowed");
        *error_code = &EC_TRADE_LONG_ONLY;
        return false;  // long only
    }
    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("AddOrder: close only allowed");
        *error_code = &EC_CLOSE_ONLY;
        return false;  // close only
    }

    //--- check secutiry
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("AddOrder: trade disabled or market closed");
        if (rt == RET_ERROR) {
            *error_code = &EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &EC_TRADE_OFFQUOTES;
        }
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
    GetCurrentPrice(symbol, user_info.group, prices, error_code);
    if (open_price <= PRICE_PRECISION) {
        if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
            *error_code = &EC_PENDING_ORDER_WITHOUT_OPEN_PRICE;
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
    if (s_interface->TradesCheckTickSize(trade_record.open_price, &symbol_cfg) == FALSE) {
        LOG("AddOrder: invalid price");
        *error_code = &EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check volume
    if (s_interface->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("AddOrder: invalid volume");
        *error_code = &EC_TRADE_BAD_VOLUME;
        return false;  // invalid volume
    }

    //--- check margin
    if (cmd == OP_BUY || cmd == OP_SELL) {
        margin = s_interface->TradesMarginCheck(&user_info, &trade_trans_info, &profit, &free_margin, &prev_margin);
        if ((free_margin + group_cfg.credit) < 0 && (symbol_cfg.margin_hedged_strong != FALSE || prev_margin <= margin)) {
            LOG("AddOrder: not enough margin");
            *error_code = &EC_TRADE_NO_MONEY;
            return false;  // no enough margin
        }
    }

    //--- prepare new trade state of order
    trade_record.login = login;
    trade_record.cmd = cmd;
    trade_record.open_price = trade_trans_info.price;
    trade_record.volume = volume;
    trade_record.close_price = ((cmd == OP_BUY || cmd == OP_BUY_LIMIT || cmd == OP_BUY_STOP) ? prices[0] : prices[1]);
    trade_record.open_time = s_interface->TradeTime();
    trade_record.digits = symbol_cfg.digits;

    COPY_STR(trade_record.symbol, symbol);
    COPY_STR(trade_record.comment, comment);

    trade_record.sl = trade_trans_info.sl;
    trade_record.tp = trade_trans_info.tp;

    //--- check stops
    if (s_interface->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("AddOrder: invalid stops");
        *error_code = &EC_TRADE_BAD_STOPS;
        return false;  // invalid stops
    }

    //--- add order into database directly
    LOG_INFO(&trade_record);
    if ((*order = s_interface->OrdersAdd(&trade_record, &user_info, &symbol_cfg)) == 0) {
        LOG("AddOrder: OrdersAdd failed");
        *error_code = &EC_UNKNOWN_ERROR;
        return false;  // error
    }

    //--- postion opened: return order
    *error_code = &EC_OK;
    return true;
}

bool ServerApi::UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, const char* comment,
                            const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};

    if (order <= 0) {
        LOG("AddOrder: invalid order");
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }

    //--- get order
    if (s_interface->OrdersGet(order, &trade_record) == FALSE) {
        LOG("UpdateOrder: OrdersGet failed");
        *error_code = &EC_INVALID_ORDER_TICKET;
        return false;  // error
    }

    //--- check state
    if (trade_record.cmd < OP_BUY || trade_record.cmd > OP_SELL_STOP) {
        LOG("UpdateOrder: invalid order command");
        *error_code = &EC_INVALID_ORDER_TICKET;
        return false;  // order already activated
    }

    //--- get user info
    if (!GetUserInfo(trade_record.login, &user_info, error_code)) {
        LOG("UpdateOrder: GetUserInfo failed");
        return false;  // error
    }

    //--- get group config
    if (s_interface->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("UpdateOrder: GroupsGet failed [%s]", user_info.group);
        *error_code = &EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(trade_record.symbol, &symbol_cfg) == FALSE) {
        LOG("UpdateOrder: SymbolsGet failed [%s]", trade_record.symbol);
        *error_code = &EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("UpdateOrder: market closed", user_info.group);
        *error_code = &EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- check tick size
    if (s_interface->TradesCheckTickSize(open_price, &symbol_cfg) == FALSE) {
        LOG("UpdateOrder: invalid price");
        *error_code = &EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("UpdateOrder: close only");
        *error_code = &EC_CLOSE_ONLY;
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
            *error_code = &EC_CHANGE_OPEN_PRICE;
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
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("UpdateOrder: trade disabled or market closed");
        if (rt == RET_ERROR) {
            *error_code = &EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &EC_TRADE_OFFQUOTES;
        }
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- check stops
    LOG_INFO(&trade_trans_info);
    if (s_interface->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("UpdateOrder: invalid stops");
        *error_code = &EC_TRADE_BAD_STOPS;
        return false;  // invalid stops
    }

    COPY_STR(trade_record.comment, comment);
    if (s_interface->OrdersUpdate(&trade_record, &user_info, UPDATE_NORMAL) == FALSE) {
        LOG("UpdateOrder: OrdersUpdate failed");
        *error_code = &EC_UNKNOWN_ERROR;
        return false;  // error
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::CloseOrder(const char* ip, const int order, double close_price, const char* comment,
                           const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConGroup group_cfg = {0};
    ConSymbol symbol_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};

    if (order <= 0) {
        LOG("CloseOrder: Invalid order");
        *error_code = &EC_BAD_PARAMETER;
        return false;
    }
    
    //--- get order
    if (s_interface->OrdersGet(order, &trade_record) == FALSE) {
        LOG("CloseOrder: OrdersGet failed");
        *error_code = &EC_INVALID_ORDER_TICKET;
        return false;  // error
    }

    //--- get user info
    if (!GetUserInfo(trade_record.login, &user_info, error_code)) {
        LOG("CloseOrder: GetUserInfo failed");
        return false;  // error
    }

    LOG_INFO(&trade_record);

    //--- get group config
    if (s_interface->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("CloseOrder: GroupsGet failed [%s]", user_info.group);
        *error_code = &EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(trade_record.symbol, &symbol_cfg) == FALSE) {
        LOG("CloseOrder: SymbolsGet failed [%s]", trade_record.symbol);
        *error_code = &EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("CloseOrder: market closed", user_info.group);
        *error_code = &EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- prepare transaction
    trade_trans_info.order = order;
    double prices[2] = {0};
    GetCurrentPrice(trade_record.symbol, user_info.group, prices, error_code);
    if (close_price <= PRICE_PRECISION) {
        close_price = (trade_record.cmd == OP_BUY || trade_record.cmd == OP_BUY_LIMIT || trade_record.cmd == OP_BUY_STOP)
                          ? prices[0]
                          : prices[1];
    }
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
    if (s_interface->TradesCheckTickSize(close_price, &symbol_cfg) == FALSE) {
        LOG("CloseOrder: invalid price");
        *error_code = &EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check secutiry
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("OrdersUpdateClose: trade disabled or market closed");
        if (rt == RET_ERROR) {
            *error_code = &EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &EC_TRADE_OFFQUOTES;
        }
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- check volume
    if (s_interface->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("OrdersUpdateClose: invalid volume");
        *error_code = &EC_TRADE_BAD_VOLUME;
        return false;  // invalid volume
    }

    //--- check stops
    if (s_interface->TradesCheckFreezed(&symbol_cfg, &group_cfg, &trade_record) != RET_OK) {
        LOG("OrdersUpdateClose: position freezed");
        *error_code = &EC_TRADE_MODIFY_DENIED;
        return false;  // position freezed
    }

    //--- close position
    LOG_INFO(&trade_trans_info);
    if (s_interface->OrdersClose(&trade_trans_info, &user_info) == FALSE) {
        LOG("CloseOrder: CloseOrder failed");
        *error_code = &EC_UNKNOWN_ERROR;
        return false;  // error
    }

    //--- position closed
    *error_code = &EC_OK;
    return true;
}

bool ServerApi::Deposit(const int login, const char* ip, const double value, const char* comment, double* balance,
                        const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user = {0};
    if (!GetUserInfo(login, &user, error_code)) {
        return false;
    }

    if ((*balance = s_interface->ClientsChangeBalance(login, &user.grp, value, comment)) == 0) {
        *error_code = &EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetUserInfo(const int login, UserInfo* user_info, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserRecord user_record = {0};

    if (s_interface->ClientsUserInfo(login, &user_record) == FALSE) {
        *error_code = &EC_INVALID_USER_ID;
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
    if (s_interface->GroupsGet(user_info->group, &grpcfg) == FALSE) {
        *error_code = &EC_GROUP_NOT_FOUND;
        return false;
    }
    user_info->grp = grpcfg;

    *error_code = &EC_OK;
    return true;
}

bool ServerApi::GetCurrentPrice(const char* symbol, const char* group, double* prices, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    ConGroup grpcfg = {0};
    if (s_interface->GroupsGet(group, &grpcfg) == FALSE) {
        *error_code = &EC_GROUP_NOT_FOUND;
        return false;
    }

    if (s_interface->HistoryPricesGroup(symbol, &grpcfg, prices) != RET_OK) {
        *error_code = &EC_GET_PRICE_ERROR;
        return false;
    }

    *error_code = &EC_OK;
    return true;
}
