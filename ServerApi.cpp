#include <process.h>
#include <stdlib.h>
#include "Config.h"
#include "ErrorCode.h"
#include "Loger.h"
#include "ServerApi.h"
#include "common.h"
#include "../include/MT4ServerAPI.h"

CServerInterface* ServerApi::s_interface = NULL;
ConSymbol ServerApi::s_symbols[MAX_SYMBOL_COUNT] = {0};
int ServerApi::s_symbol_count = 0;
Synchronizer ServerApi::s_deposit_sync;

void ServerApi::Initialize(CServerInterface* server_interface) {
    s_interface = server_interface;
}

CServerInterface* ServerApi::Api() {
    return s_interface;
}

bool ServerApi::OpenOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                          double sl, double tp, time_t expiration, const char* comment, const ErrorCode** error_code,
                          int* order) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};

    //--- checks
    if (login <= 0 || cmd < OP_BUY || cmd > OP_SELL_STOP || symbol == NULL || volume <= 0) {
        LOG("OpenOrder: Invalid parameters");
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
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
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(symbol, &symbol_cfg) == FALSE) {
        LOG("OpenOrder: SymbolsGet failed [%s]", symbol);
        *error_code = &ErrorCode::EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("AddOrder: market closed", user_info.group);
        *error_code = &ErrorCode::EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- check long only
    if (symbol_cfg.long_only != FALSE && (cmd == OP_SELL || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP)) {
        LOG("OpenOrder: long only allowed");
        *error_code = &ErrorCode::EC_TRADE_LONG_ONLY;
        return false;  // long only
    }

    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("OpenOrder: close only allowed");
        *error_code = &ErrorCode::EC_CLOSE_ONLY;
        return false;  // close only
    }

    //--- check secutiry
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("OpenOrder: trade disabled or market closed %d", rt);
        if (rt == RET_ERROR) {
            *error_code = &ErrorCode::EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &ErrorCode::EC_TRADE_DISABLE;
        } else {
            *error_code = &ErrorCode::EC_TRADE_OFFQUOTES;
        }
        return false;
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

    if (expiration != -1 && cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
        trade_trans_info.expiration = expiration;
    }

    double prices[2] = {0};
    GetCurrentPrice(symbol, user_info.group, prices, error_code);
    if (open_price <= PRICE_PRECISION) {
        if (cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
            *error_code = &ErrorCode::EC_PENDING_ORDER_WITHOUT_OPEN_PRICE;
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
        *error_code = &ErrorCode::EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check volume
    if (s_interface->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("OpenOrder: invalid volume");
        *error_code = &ErrorCode::EC_TRADE_BAD_VOLUME;
        return false;  // invalid volume
    }

    //--- check margin
    if (cmd == OP_BUY || cmd == OP_SELL) {
        double profit = 0, margin = 0, free_margin = 0, prev_margin = 0;
        margin = s_interface->TradesMarginCheck(&user_info, &trade_trans_info, &profit, &free_margin, &prev_margin);
        if ((free_margin + group_cfg.credit) < 0 && (symbol_cfg.margin_hedged_strong != FALSE || prev_margin <= margin)) {
            LOG("OpenOrder: not enough margin");
            *error_code = &ErrorCode::EC_TRADE_NO_MONEY;
            return false;  // no enough margin
        }
    }

    //--- check stops
    if (s_interface->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("OpenOrder: invalid stops");
        *error_code = &ErrorCode::EC_TRADE_BAD_STOPS;
        return false;  // invalid stops
    }

    //--- check off quote
    if ((cmd == OP_BUY || cmd == OP_SELL) && !IsQuoteAlive(symbol, error_code)) {
        LOG("OpenOrder: quote interruped");
        return false;
    }

    //--- open order with margin check
    // LOG_INFO(&trade_trans_info);
    if ((*order = s_interface->OrdersOpen(&trade_trans_info, &user_info)) == 0) {
        LOG("OpenOrder: OpenOrder failed");
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;  // error
    }

    //--- postion opened: return order
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetUserRecord(int user, UserRecord* user_record, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user_record == NULL || user < 0) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->ClientsUserInfo(user, user_record) == FALSE) {
        *error_code = &ErrorCode::EC_INVALID_USER_ID;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::UpdateUserRecord(int user, const char* group, const char* name, const char* phone, const char* email,
                                 int enable, int leverage, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    UserRecord user_record = {0};

    if (s_interface->ClientsUserInfo(user, &user_record) == FALSE) {
        *error_code = &ErrorCode::EC_INVALID_USER_ID;
        return false;
    }

    if (group != NULL && strnlen_s(group, sizeof(user_record.group)) != 0) {
        COPY_STR(user_record.group, group);
    }
    if (name != NULL && strnlen_s(name, sizeof(user_record.name)) != 0) {
        COPY_STR(user_record.name, name);
    }
    if (phone != NULL && strnlen_s(phone, sizeof(user_record.phone)) != 0) {
        COPY_STR(user_record.phone, phone);
    }
    if (email != NULL && strnlen_s(email, sizeof(user_record.email)) != 0) {
        COPY_STR(user_record.email, email);
    }
    if (enable != -1) {
        user_record.enable = enable;
    }
    if (leverage != -1) {
        user_record.leverage = leverage;
    }
    if (s_interface->ClientsUserUpdate(&user_record) == FALSE) {
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::ChangePassword(int user, const char* password, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (strnlen_s(password, 32) < 5) {
        *error_code = &ErrorCode::EC_WRONG_PASSWORD;
        return false;
    }

    if (s_interface->ClientsChangePass(user, password, FALSE, FALSE) == FALSE) {
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::CheckPassword(int user, const char* password, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->ClientsCheckPass(user, password, FALSE) == FALSE) {
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetMargin(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity,
                          const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0 || user_info == NULL || margin == NULL || freemargin == NULL || equity == NULL) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->TradesMarginGet(user, user_info, margin, freemargin, equity) == FALSE) {
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetMarginInfo(int user, UserInfo* user_info, double* margin, double* freemargin, double* equity,
                              const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0 || user_info == NULL || margin == NULL || freemargin == NULL || equity == NULL) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (GetUserInfo(user, user_info, error_code) == FALSE) {
        return false;
    }

    if (s_interface->TradesMarginInfo(user_info, margin, freemargin, equity) == FALSE) {
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetOrder(int order, TradeRecord* trade_record, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (order < 0 || trade_record == NULL) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (s_interface->OrdersGet(order, trade_record) == FALSE) {
        *error_code = &ErrorCode::EC_INVALID_ORDER_TICKET;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetOpenOrders(int user, int* total, TradeRecord** orders, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    UserInfo user_info = {0};
    if (GetUserInfo(user, &user_info, error_code) == FALSE) {
        return false;
    }

    *orders = s_interface->OrdersGetOpen(&user_info, total);
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetClosedOrders(int user, time_t from, time_t to, int* total, TradeRecord** orders,
                                const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (user < 0) {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }
    if (from == -1) {
        from = 0;
    }
    if (to == -1) {
        to = s_interface->TradeTime();
    }

    int users[1] = {user};
    *orders = s_interface->OrdersGetClosed(from, to, users, 1, total);
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::IsOpening(const char* symbol, time_t time, bool* result, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    if (time == 0) {
        time = s_interface->TradeTime();
    }

    ConSymbol con_symbol = {0};
    if (s_interface->SymbolsGet(symbol, &con_symbol) == FALSE) {
        *result = false;
        *error_code = &ErrorCode::EC_SYMBOL_NOT_FOUND;
        return false;
    }

    *result = (s_interface->TradesCheckSessions(&con_symbol, time) == TRUE);
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::CurrentTradeTime(time_t* time, const ErrorCode** error_code) {
    FUNC_WARDER;

    *time = s_interface->TradeTime();
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetSymbolList(int* total, const ConSymbol** const symbols, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_symbol_count == 0) {
        UpdateSymbolList();
    }

    *total = s_symbol_count;
    *symbols = s_symbols;
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::AddUser(int login, const char* name, const char* password, const char* group, const char* phone,
                        const char* email, const char* lead_source, int leverage, const ErrorCode** error_code, int* user) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserRecord user_record = {0};
    ConGroup group_cfg = {0};

    if (login <= 0) {
        *error_code = &ErrorCode::EC_INVALID_USER_ID;
        return false;
    }

    if (s_interface->ClientsUserInfo(login, &user_record) == TRUE) {
        *error_code = &ErrorCode::EC_USER_ID_EXISTING;
        return false;
    }

    user_record.login = login;

    if (name != NULL && strnlen_s(name, sizeof(user_record.name)) != 0) {
        COPY_STR(user_record.name, name);
    } else {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (password != NULL && strnlen_s(password, sizeof(user_record.password)) != 0) {
        COPY_STR(user_record.password, password);
    } else {
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    if (group != NULL && strnlen_s(group, sizeof(user_record.group)) != 0 &&
        s_interface->GroupsGet(group, &group_cfg) == TRUE) {
        COPY_STR(user_record.group, group);
    } else {
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;
    }

    if (phone != NULL && strnlen_s(phone, sizeof(user_record.phone)) != 0) {
        COPY_STR(user_record.phone, phone);
    }

    if (email != NULL && strnlen_s(email, sizeof(user_record.email)) != 0) {
        COPY_STR(user_record.email, email);
    }

    if (lead_source != NULL && strnlen_s(lead_source, sizeof(user_record.lead_source)) != 0) {
        COPY_STR(user_record.lead_source, lead_source);
    }

    // user is enabled as default
    user_record.enable = 1;

    user_record.leverage = leverage > 0 ? leverage : group_cfg.default_leverage;

    if (s_interface->ClientsAddUser(&user_record) == FALSE) {
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;
    }

    *user = user_record.login;
    *error_code = &ErrorCode::EC_OK;
    return true;
}

void ServerApi::SymbolChanged() {
    s_symbol_count = 0;
}

bool ServerApi::UpdateSymbolList() {
    FUNC_WARDER;

    s_symbol_count = 0;
    while (s_interface->SymbolsNext(s_symbol_count, &s_symbols[s_symbol_count]) != FALSE) {
        s_symbol_count++;
    }
    return true;
}

bool ServerApi::AddOrder(const int login, const char* ip, const char* symbol, const int cmd, int volume, double open_price,
                         double sl, double tp, time_t expiration, const char* comment, const ErrorCode** error_code,
                         int* order) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
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
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
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
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(symbol, &symbol_cfg) == FALSE) {
        LOG("AddOrder: SymbolsGet failed [%s]", symbol);
        *error_code = &ErrorCode::EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("AddOrder: market closed", user_info.group);
        *error_code = &ErrorCode::EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- check long only
    if (symbol_cfg.long_only != FALSE && (cmd == OP_SELL || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP)) {
        LOG("AddOrder: long only allowed");
        *error_code = &ErrorCode::EC_TRADE_LONG_ONLY;
        return false;  // long only
    }
    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("AddOrder: close only allowed");
        *error_code = &ErrorCode::EC_CLOSE_ONLY;
        return false;  // close only
    }

    //--- check secutiry
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("AddOrder: trade disabled or market closed");
        if (rt == RET_ERROR) {
            *error_code = &ErrorCode::EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &ErrorCode::EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &ErrorCode::EC_TRADE_OFFQUOTES;
        } else {
            *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
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
            *error_code = &ErrorCode::EC_PENDING_ORDER_WITHOUT_OPEN_PRICE;
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
        *error_code = &ErrorCode::EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check volume
    if (s_interface->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("AddOrder: invalid volume");
        *error_code = &ErrorCode::EC_TRADE_BAD_VOLUME;
        return false;  // invalid volume
    }

    //--- check margin
    if (cmd == OP_BUY || cmd == OP_SELL) {
        margin = s_interface->TradesMarginCheck(&user_info, &trade_trans_info, &profit, &free_margin, &prev_margin);
        LOG("AddOrder: TradesMarginCheck  margin = %d, profit = %d, free_margin = %d, prev_margin = %d", margin, profit,
            free_margin, prev_margin);
        if ((free_margin + group_cfg.credit) < 0 && (symbol_cfg.margin_hedged_strong != FALSE || prev_margin <= margin)) {
            LOG("AddOrder: not enough margin");
            *error_code = &ErrorCode::EC_TRADE_NO_MONEY;
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

    if (expiration != -1 && cmd >= OP_BUY_LIMIT && cmd <= OP_SELL_STOP) {
        trade_record.expiration = expiration;
    }

    //--- check stops
    if (s_interface->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("AddOrder: invalid stops");
        *error_code = &ErrorCode::EC_TRADE_BAD_STOPS;
        return false;  // invalid stops
    }

    //--- check off quote
    if ((cmd == OP_BUY || cmd == OP_SELL) && !IsQuoteAlive(symbol, error_code)) {
        LOG("AddOrder: quote interrupted");
        return false;
    }

    //--- add order into database directly
    // LOG_INFO(&trade_record);
    if ((*order = s_interface->OrdersAdd(&trade_record, &user_info, &symbol_cfg)) == 0) {
        LOG("AddOrder: OrdersAdd failed");
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;  // error
    }

    //--- postion opened: return order
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::UpdateOrder(const char* ip, const int order, double open_price, double sl, double tp, time_t expiration,
                            const char* comment, const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConSymbol symbol_cfg = {0};
    ConGroup group_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};

    if (order <= 0) {
        LOG("AddOrder: invalid order");
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    //--- get order
    if (s_interface->OrdersGet(order, &trade_record) == FALSE) {
        LOG("UpdateOrder: OrdersGet failed");
        *error_code = &ErrorCode::EC_INVALID_ORDER_TICKET;
        return false;  // error
    }

    //--- check state
    if (trade_record.cmd < OP_BUY || trade_record.cmd > OP_SELL_STOP) {
        LOG("UpdateOrder: invalid order command");
        *error_code = &ErrorCode::EC_INVALID_ORDER_TICKET;
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
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(trade_record.symbol, &symbol_cfg) == FALSE) {
        LOG("UpdateOrder: SymbolsGet failed [%s]", trade_record.symbol);
        *error_code = &ErrorCode::EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("UpdateOrder: market closed", user_info.group);
        *error_code = &ErrorCode::EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    //--- check tick size
    if (s_interface->TradesCheckTickSize(open_price, &symbol_cfg) == FALSE) {
        LOG("UpdateOrder: invalid price");
        *error_code = &ErrorCode::EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check close only
    if (symbol_cfg.trade == TRADE_CLOSE) {
        LOG("UpdateOrder: close only");
        *error_code = &ErrorCode::EC_CLOSE_ONLY;
        return false;
    }

    if (sl - trade_record.sl > PRICE_PRECISION || trade_record.sl - sl > PRICE_PRECISION) {
        trade_trans_info.sl = sl;
        trade_record.sl = sl;
        LOG("UpdateOrder: sl is set to %f", sl);
    }

    if (tp - trade_record.tp > PRICE_PRECISION || trade_record.tp - tp > PRICE_PRECISION) {
        trade_trans_info.tp = tp;
        trade_record.tp = tp;
        LOG("UpdateOrder: tp is set to %f", tp);
    }

    if (open_price - trade_record.open_price > PRICE_PRECISION || trade_record.open_price - open_price > PRICE_PRECISION) {
        // modification of open price is forbidden
        if (trade_record.cmd == OP_BUY || trade_record.cmd == OP_SELL) {
            LOG("UpdateOrder: modification of open price [%f] is forbidden", open_price);
            *error_code = &ErrorCode::EC_CHANGE_OPEN_PRICE;
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
            *error_code = &ErrorCode::EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &ErrorCode::EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &ErrorCode::EC_TRADE_OFFQUOTES;
        } else {
            *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        }
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- check stops
    // LOG_INFO(&trade_trans_info);
    if (s_interface->TradesCheckStops(&trade_trans_info, &symbol_cfg, &group_cfg, NULL) != RET_OK) {
        LOG("UpdateOrder: invalid stops");
        *error_code = &ErrorCode::EC_TRADE_BAD_STOPS;
        return false;  // invalid stops
    }

    if (expiration != -1 && trade_record.cmd >= OP_BUY_LIMIT && trade_record.cmd <= OP_SELL_STOP) {
        trade_record.expiration = expiration;
    }

    COPY_STR(trade_record.comment, comment);
    if (s_interface->OrdersUpdate(&trade_record, &user_info, UPDATE_NORMAL) == FALSE) {
        LOG("UpdateOrder: OrdersUpdate failed");
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;  // error
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::CloseOrder(const char* ip, const int order, double close_price, const char* comment,
                           const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserInfo user_info = {0};
    ConGroup group_cfg = {0};
    ConSymbol symbol_cfg = {0};
    TradeTransInfo trade_trans_info = {0};
    TradeRecord trade_record = {0};

    if (order <= 0) {
        LOG("CloseOrder: Invalid order");
        *error_code = &ErrorCode::EC_BAD_PARAMETER;
        return false;
    }

    //--- get order
    if (s_interface->OrdersGet(order, &trade_record) == FALSE) {
        LOG("CloseOrder: OrdersGet failed");
        *error_code = &ErrorCode::EC_INVALID_ORDER_TICKET;
        return false;  // error
    }

    //--- get user info
    if (!GetUserInfo(trade_record.login, &user_info, error_code)) {
        LOG("CloseOrder: GetUserInfo failed");
        return false;  // error
    }

    // LOG_INFO(&trade_record);

    //--- get group config
    if (s_interface->GroupsGet(user_info.group, &group_cfg) == FALSE) {
        LOG("CloseOrder: GroupsGet failed [%s]", user_info.group);
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;  // error
    }

    //--- get symbol config
    if (s_interface->SymbolsGet(trade_record.symbol, &symbol_cfg) == FALSE) {
        LOG("CloseOrder: SymbolsGet failed [%s]", trade_record.symbol);
        *error_code = &ErrorCode::EC_SYMBOL_NOT_FOUND;
        return false;  // error
    }

    if (s_interface->TradesCheckSessions(&symbol_cfg, s_interface->TradeTime()) == FALSE) {
        LOG("CloseOrder: market closed", user_info.group);
        *error_code = &ErrorCode::EC_TRADE_MARKET_CLOSED;
        return false;  // error
    }

    // check offquote
    if (!IsQuoteAlive(trade_record.symbol, error_code)) {
        LOG("CloseOrder: quote interruped");
        return false;
    }

    if (trade_record.cmd >= OP_BUY_LIMIT && trade_record.cmd <= OP_SELL_STOP) {
        LOG("CloseOrder: delete pending order");
        COPY_STR(trade_record.comment, comment);
        trade_record.close_time = s_interface->TradeTime();
        if (s_interface->OrdersUpdate(&trade_record, &user_info, UPDATE_CLOSE) == FALSE) {
            LOG("CloseOrder: CloseOrder failed");
            *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
            return false;  // error
        } else {
            *error_code = &ErrorCode::EC_OK;
            return true;
        }
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
    trade_trans_info.type = TT_ORDER_MK_CLOSE;

    //--- check tick size
    if (s_interface->TradesCheckTickSize(close_price, &symbol_cfg) == FALSE) {
        LOG("CloseOrder: invalid price");
        *error_code = &ErrorCode::EC_TRADE_BAD_PRICES;
        return false;  // invalid price
    }

    //--- check secutiry
    if (int rt = s_interface->TradesCheckSecurity(&symbol_cfg, &group_cfg) != RET_OK) {
        LOG("OrdersUpdateClose: trade disabled or market closed %d", rt);
        if (rt == RET_ERROR) {
            *error_code = &ErrorCode::EC_BAD_PARAMETER;
        } else if (rt == RET_TRADE_DISABLE) {
            *error_code = &ErrorCode::EC_TRADE_DISABLE;
        } else if (rt == RET_TRADE_OFFQUOTES) {
            *error_code = &ErrorCode::EC_TRADE_OFFQUOTES;
        } else {
            *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        }
        return false;  // trade disabled, market closed, or no prices for long time
    }

    //--- check volume
    if (s_interface->TradesCheckVolume(&trade_trans_info, &symbol_cfg, &group_cfg, TRUE) != RET_OK) {
        LOG("OrdersUpdateClose: invalid volume");
        *error_code = &ErrorCode::EC_TRADE_BAD_VOLUME;
        return false;  // invalid volume
    }

    //--- check stops
    if (s_interface->TradesCheckFreezed(&symbol_cfg, &group_cfg, &trade_record) != RET_OK) {
        LOG("OrdersUpdateClose: position freezed");
        *error_code = &ErrorCode::EC_TRADE_MODIFY_DENIED;
        return false;  // position freezed
    }

    //--- close position
    // LOG_INFO(&trade_trans_info);
    if (s_interface->OrdersClose(&trade_trans_info, &user_info) == FALSE) {
        LOG("CloseOrder: CloseOrder failed");
        *error_code = &ErrorCode::EC_UNKNOWN_ERROR;
        return false;  // error
    }

    //--- position closed
    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::Deposit(const int login, const char* ip, const double value, const char* comment, int* order,
                        const ErrorCode** error_code) {
    FUNC_WARDER;

    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    s_deposit_sync.Lock();

    if (comment != nullptr && comment[0] >= '0' && comment[0] <= '9') {
        int total = 0;
        TradeRecord* trade_record = NULL;
        bool handled = false;
        bool result = ServerApi::GetClosedOrders(login, ServerApi::Api()->TradeTime() - 3600, ServerApi::Api()->TradeTime(),
                                                 &total, &trade_record, error_code);

        if (result && trade_record != NULL) {
            for (int i = 0; i < total; ++i) {
                TradeRecord* trade = trade_record + i;
                if (trade->cmd == OP_BALANCE && strcmp(comment, trade->comment) == 0) {
                    handled = true;
                    break;
                }
            }
        }

        if (trade_record != NULL) {
            HEAP_FREE(trade_record);
            trade_record = NULL;
        }

        if (handled) {
            *error_code = &ErrorCode::EC_ALREADY_DEPOSIT;
            s_deposit_sync.Unlock();
            return false;
        }
    }

    UserInfo user = {0};
    if (!GetUserInfo(login, &user, error_code)) {
        s_deposit_sync.Unlock();
        return false;
    }

    *order = s_interface->ClientsChangeBalance(login, &user.grp, value, comment);
    s_deposit_sync.Unlock();
    *error_code = (*order == 0) ? &ErrorCode::EC_UNKNOWN_ERROR : &ErrorCode::EC_OK;
    return (*order != 0);
}

bool ServerApi::GetUserInfo(const int login, UserInfo* user_info, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    UserRecord user_record = {0};

    if (s_interface->ClientsUserInfo(login, &user_record) == FALSE) {
        *error_code = &ErrorCode::EC_INVALID_USER_ID;
        return false;
    }

    user_info->balance = user_record.balance;
    user_info->login = user_record.login;
    user_info->credit = user_record.credit;
    user_info->enable = user_record.enable;
    user_info->leverage = user_record.leverage;
    COPY_STR(user_info->group, user_record.group);
    COPY_STR(user_info->name, user_record.name);

    if (s_interface->GroupsGet(user_info->group, &user_info->grp) == FALSE) {
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::GetCurrentPrice(const char* symbol, const char* group, double* prices, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    ConGroup grpcfg = {0};
    if (s_interface->GroupsGet(group, &grpcfg) == FALSE) {
        *error_code = &ErrorCode::EC_GROUP_NOT_FOUND;
        return false;
    }

    if (s_interface->HistoryPricesGroup(symbol, &grpcfg, prices) != RET_OK) {
        *error_code = &ErrorCode::EC_GET_PRICE_ERROR;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}

bool ServerApi::IsQuoteAlive(const char* symbol, const ErrorCode** error_code) {
    if (s_interface == NULL) {
        *error_code = &ErrorCode::EC_INVALID_SERVER_INTERFACE;
        return false;
    }

    time_t last_time;
    double prices[2];
    int direction;
    if (s_interface->HistoryPrices(symbol, prices, &last_time, &direction) != RET_OK) {
        LOG("IsQuoteAlive: HistoryPrices failed");
        *error_code = &ErrorCode::EC_TRADE_OFFQUOTES;
        return false;
    }

    int offquote_time = 300;
    if (Config::Instance().HasKey(symbol)) {
        Config::Instance().GetInteger(symbol, &offquote_time, "300");
    } else {
        Config::Instance().GetInteger("default off quote time", &offquote_time, "300");
    }

    if (s_interface->TradeTime() - last_time > offquote_time) {
        LOG("IsQuoteAlive: last quote is too old. trade_time: %d, quote_time: %d, offquote_time: %d", s_interface->TradeTime(),
            last_time, offquote_time);
        *error_code = &ErrorCode::EC_TRADE_OFFQUOTES;
        return false;
    }

    *error_code = &ErrorCode::EC_OK;
    return true;
}
