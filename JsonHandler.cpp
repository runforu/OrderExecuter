
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "ErrorCode.h"
#include "JsonHandler.h"
#include "JsonWrapper.h"
#include "LicenseService.h"
#include "Loger.h"
#include "MT4ServerAPI.h"
#include "ServerApi.h"
#include "common.h"
#include "http_server/connection.h"

using namespace boost::property_tree;

int JsonHandler::get_priority() const {
    return 100;
}

bool JsonHandler::can_handle(const http::server::request& req) const {
    return std::any_of(req.headers.begin(), req.headers.end(), [](const http::server::header& h) { return h.name == "content-type" && h.value == "application/json"; });
}

bool JsonHandler::handle(const http::server::request& req, http::server::reply& rep) const {
    ptree json;
    JsonWrapper::ParseJson(req.body, json);

#ifdef _LICENSE_VERIFICATION_
    if (!LicenseService::Instance().IsLicenseValid()) {
        rep.status = http::server::reply::bad_request;
        ptree response;
        response.put("json_error", "Invalid plugin license");
        rep.content.append(JsonWrapper::ToJsonStr(response));
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Type";
        rep.headers[0].value = "application/json";
        rep.headers[1].name = "Content-Length";
        rep.headers[1].value = std::to_string(rep.content.length());
        return true;
    }
#endif

    if (!json.empty()) {
        rep.status = http::server::reply::ok;
        std::string request = json.get<std::string>("request", "");
        if (request.compare("Ping") == 0) {
            Ping(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("BinaryOption") == 0) {
            BinaryOption(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("OpenOrder") == 0) {
            OpenOrder(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("AddOrder") == 0) {
            AddOrder(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("UpdateOrder") == 0) {
            UpdateOrder(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("CloseOrder") == 0) {
            CloseOrder(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("Deposit") == 0) {
            Deposit(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("GetUserRecord") == 0) {
            GetUserRecord(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("UpdateUserRecord") == 0) {
            UpdateUserRecord(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("AddUser") == 0) {
            AddUser(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("ChangePassword") == 0) {
            ChangePassword(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("CheckPassword") == 0) {
            CheckPassword(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("GetMargin") == 0) {
            GetMargin(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("GetMarginInfo") == 0) {
            GetMargin(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("GetOrder") == 0) {
            GetOrder(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("GetOpenOrders") == 0) {
            GetOpenOrders(json, rep.content);
        } else if (request.compare("GetPendingOrders") == 0) {
            GetPendingOrders(json, rep.content);
        } else if (request.compare("GetClosedOrders") == 0) {
            GetClosedOrders(json, rep.content);
        } else if (request.compare("IsOpening") == 0) {
            IsOpening(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("TradeTime") == 0) {
            TradeTime(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else if (request.compare("GetSymbolList") == 0) {
            GetSymbolList(json);
            rep.content.append(JsonWrapper::ToJsonStr(json));
        } else {
            return false;
        }
    } else {
        // Handle json parse error
        rep.status = http::server::reply::bad_request;
        ptree response;
        response.put("json_error", "Invalid json format");
        rep.content.append(JsonWrapper::ToJsonStr(response));
    }

    rep.headers.resize(4);
    rep.headers[0].name = "Content-Type";
    rep.headers[0].value = "application/json";
    rep.headers[1].name = "Connection";
    rep.headers[1].value = "keep-alive";
    rep.headers[2].name = "Keep-Alive";
    rep.headers[2].value = "timeout=180";
    rep.headers[3].name = "Content-Length";
    rep.headers[3].value = std::to_string(rep.content.length());

    return true;
}

void JsonHandler::Ping(boost::property_tree::ptree& pt) const {
    SetResponseJson(pt, "Ping", true, &ErrorCode::EC_OK);
    pt.put("connections", http::server::connection::total_connection());
}

void JsonHandler::BinaryOption(boost::property_tree::ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    std::string symbol = pt.get<std::string>("symbol", "");
    int cmd = ToCmd(pt.get<std::string>("cmd", "OP_BUY"));
    int volume = pt.get<int>("volume", 1);
    double open_price = pt.get<double>("open_price", 0.0);
    double close_price = pt.get<double>("close_price", 0.0);
    double profit = pt.get<double>("profit", 0.0);
    std::string comment = pt.get<std::string>("comment", "BinaryOption Order");
    int order;
    const ErrorCode* error_code;

    bool result = ServerApi::BinaryOption(login, ip.c_str(), symbol.c_str(), cmd, volume, open_price, close_price, profit, comment.c_str(), &error_code, &order);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("order", order);
    }
}

void JsonHandler::OpenOrder(boost::property_tree::ptree& pt) const {
    const ErrorCode* error_code;
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");

    double coupon = pt.get<double>("coupon", 0);
    std::string coupon_comment = pt.get<std::string>("coupon_comment", "");

    int order = 0;

    if (coupon > 0.0) {
        bool result = ServerApi::Deposit(login, ip.c_str(), coupon, coupon_comment.c_str(), &order, &error_code);
        if (!result) {
            pt.clear();
            SetResponseJson(pt, request, result, error_code);
            return;
        }
    }

    std::string symbol = pt.get<std::string>("symbol", "");
    int cmd = ToCmd(pt.get<std::string>("cmd", ""));
    int volume = pt.get<int>("volume", 0);
    double open_price = pt.get<double>("open_price", 0.0);
    double sl = pt.get<double>("sl", 0.0);
    double tp = pt.get<double>("tp", 0.0);
    time_t expiration = pt.get<time_t>("expiration", -1);
    std::string comment = pt.get<std::string>("comment", "");

    bool result = ServerApi::OpenOrder(login, ip.c_str(), symbol.c_str(), cmd, volume, open_price, sl, tp, expiration, comment.c_str(), &error_code, &order);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("order", order);
    }
}

void JsonHandler::AddOrder(boost::property_tree::ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    std::string symbol = pt.get<std::string>("symbol", "");
    int cmd = ToCmd(pt.get<std::string>("cmd", ""));
    int volume = pt.get<int>("volume", 0);
    double open_price = pt.get<double>("open_price", 0.0);
    double sl = pt.get<double>("sl", 0.0);
    double tp = pt.get<double>("tp", 0.0);
    time_t expiration = pt.get<time_t>("expiration", -1);
    std::string comment = pt.get<std::string>("comment", "");
    int order;
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::AddOrder(login, ip.c_str(), symbol.c_str(), cmd, volume, open_price, sl, tp, expiration, comment.c_str(), &error_code, &order);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("order", order);
    }
}

void JsonHandler::UpdateOrder(boost::property_tree::ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    int order = pt.get<int>("order", 0);
    double open_price = pt.get<double>("open_price", 0.0);
    double sl = pt.get<double>("sl", 0.0);
    double tp = pt.get<double>("tp", 0.0);
    time_t expiration = pt.get<time_t>("expiration", -1);
    std::string comment = pt.get<std::string>("comment", "");
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::UpdateOrder(ip.c_str(), order, open_price, sl, tp, expiration, comment.c_str(), &error_code);
    pt.clear();
    SetResponseJson(pt, request, result, error_code);
}

void JsonHandler::CloseOrder(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    int order = pt.get<int>("order", 0);
    double close_price = pt.get<double>("close_price", 0.0);
    std::string comment = pt.get<std::string>("comment", "");
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::CloseOrder(ip.c_str(), order, close_price, comment.c_str(), &error_code);
    pt.clear();
    SetResponseJson(pt, request, result, error_code);
}

void JsonHandler::Deposit(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    double value = pt.get<double>("value", 0.0);
    std::string comment = pt.get<std::string>("comment", "");

    const ErrorCode* error_code = nullptr;
    int order = 0;
    bool result = ServerApi::Deposit(login, ip.c_str(), value, comment.c_str(), &order, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("order", order);
    }
}

void JsonHandler::GetUserRecord(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int user = pt.get<int>("user", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    const ErrorCode* error_code = nullptr;

    UserRecord user_record;
    bool result = ServerApi::GetUserRecord(user, &user_record, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("login", user_record.login);
        pt.put("group", user_record.group);
        pt.put("enable", user_record.enable);
        pt.put("enable_change_password", user_record.enable_change_password);
        pt.put("enable_read_only", user_record.enable_read_only);
        pt.put("enable_otp", user_record.enable_otp);
        pt.put("name", user_record.name);
        pt.put("country", user_record.country);
        pt.put("city", user_record.city);
        pt.put("state", user_record.state);
        pt.put("zipcode", user_record.zipcode);
        pt.put("address", user_record.address);
        pt.put("lead_source", user_record.lead_source);
        pt.put("phone", user_record.phone);
        pt.put("email", user_record.email);
        pt.put("comment", user_record.comment);
        pt.put("id", user_record.id);
        pt.put("status", user_record.status);
        pt.put("regdate", user_record.regdate);
        pt.put("lastdate", user_record.lastdate);
        pt.put("leverage", user_record.leverage);
        pt.put("agent_account", user_record.agent_account);
        pt.put("timestamp", user_record.timestamp);
        pt.put("last_ip", user_record.last_ip);
        pt.put("balance", user_record.balance);
        pt.put("prevmonthbalance", user_record.prevmonthbalance);
        pt.put("prevbalance", user_record.prevbalance);
        pt.put("credit", user_record.credit);
        pt.put("interestrate", user_record.interestrate);
        pt.put("taxes", user_record.taxes);
        pt.put("prevmonthequity", user_record.prevmonthequity);
        pt.put("prevequity", user_record.prevequity);
        pt.put("otp_secret", user_record.otp_secret);
        pt.put("send_reports", user_record.send_reports);
        pt.put("mqid", user_record.mqid);
        pt.put("user_color", user_record.user_color);
    }
}

void JsonHandler::UpdateUserRecord(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int user = pt.get<int>("user", -1);
    std::string group = pt.get<std::string>("group", "");
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    std::string name = pt.get<std::string>("name", "");
    std::string phone = pt.get<std::string>("phone", "");
    std::string email = pt.get<std::string>("email", "");
    int enable = pt.get<int>("enable", -1);
    int leverage = pt.get<int>("leverage", -1);
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::UpdateUserRecord(user, group.c_str(), name.c_str(), phone.c_str(), email.c_str(), enable, leverage, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
}

void JsonHandler::AddUser(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string name = pt.get<std::string>("name", "");
    std::string group = pt.get<std::string>("group", "");
    std::string password = pt.get<std::string>("password", "");
    std::string phone = pt.get<std::string>("phone", "");
    std::string email = pt.get<std::string>("email", "");
    int leverage = pt.get<int>("leverage", -1);
    std::string lead_source = pt.get<std::string>("lead_source", "");
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::AddUser(login, name.c_str(), password.c_str(), group.c_str(), phone.c_str(), email.c_str(), lead_source.c_str(), leverage, &error_code, &login);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    pt.put("login", login);
}

void JsonHandler::ChangePassword(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string password = pt.get<std::string>("password", "");
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::ChangePassword(login, password.c_str(), &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
}

void JsonHandler::CheckPassword(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string password = pt.get<std::string>("password", "");
    const ErrorCode* error_code = nullptr;

    bool result = ServerApi::CheckPassword(login, password.c_str(), &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
}

void JsonHandler::GetMargin(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    const ErrorCode* error_code = nullptr;
    UserInfo user_info = {0};
    double margin;
    double freemargin;
    double equity;

    bool result = ServerApi::GetMargin(login, &user_info, &margin, &freemargin, &equity, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("margin", margin);
        pt.put("freemargin", freemargin);
        pt.put("equity", equity);
        pt.put("group", user_info.group);
        pt.put("margin_call", user_info.grp.margin_call);
        pt.put("margin_mode", user_info.grp.margin_mode);
        pt.put("margin_stopout", user_info.grp.margin_stopout);
        pt.put("margin_type", user_info.grp.margin_type);
        pt.put("margin_stopout", user_info.grp.margin_stopout);
        pt.put("ip", user_info.ip);
        pt.put("leverage", user_info.leverage);
        pt.put("balance", user_info.balance);
        pt.put("credit", user_info.credit);
    }
}

void JsonHandler::GetMarginInfo(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    const ErrorCode* error_code = nullptr;
    UserInfo user_info = {0};
    double margin;
    double freemargin;
    double equity;

    bool result = ServerApi::GetMarginInfo(login, &user_info, &margin, &freemargin, &equity, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("margin", margin);
        pt.put("freemargin", freemargin);
        pt.put("equity", equity);
        pt.put("group", user_info.group);
        pt.put("margin_call", user_info.grp.margin_call);
        pt.put("margin_mode", user_info.grp.margin_mode);
        pt.put("margin_stopout", user_info.grp.margin_stopout);
        pt.put("margin_type", user_info.grp.margin_type);
        pt.put("margin_stopout", user_info.grp.margin_stopout);
        pt.put("ip", user_info.ip);
        pt.put("leverage", user_info.leverage);
        pt.put("balance", user_info.balance);
        pt.put("credit", user_info.credit);
    }
}

void JsonHandler::GetOrder(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int order = pt.get<int>("order", -1);
    const ErrorCode* error_code = nullptr;
    TradeRecord trade_record = {0};

    bool result = ServerApi::GetOrder(order, &trade_record, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    if (result) {
        pt.put("order", trade_record.order);
        pt.put("login", trade_record.login);
        pt.put("symbol", trade_record.symbol);
        pt.put("digits", trade_record.digits);
        pt.put("cmd", trade_record.cmd);
        pt.put("volume", trade_record.volume);
        pt.put("open_time", trade_record.open_time);
        pt.put("state", ToTradeRecordStateStr(trade_record.state));
        pt.put("open_price", trade_record.open_price);
        pt.put("sl", trade_record.sl);
        pt.put("tp", trade_record.tp);
        pt.put("close_time", trade_record.close_time);
        pt.put("gw_volume", trade_record.gw_volume);
        pt.put("expiration", trade_record.expiration);
        pt.put("commission", trade_record.commission);
        pt.put("commission_agent", trade_record.commission_agent);
        pt.put("storage", trade_record.storage);
        pt.put("close_price", trade_record.close_price);
        pt.put("profit", NormalizeDouble(trade_record.profit, 2));
        pt.put("taxes", trade_record.taxes);
        pt.put("magic", trade_record.magic);
        pt.put("comment", trade_record.comment);
        pt.put("gw_order", trade_record.gw_order);
        pt.put("activation", trade_record.activation);
        pt.put("gw_open_price", trade_record.gw_open_price);
        pt.put("gw_close_price", trade_record.gw_close_price);
        pt.put("margin_rate", trade_record.margin_rate);
        pt.put("timestamp", trade_record.timestamp);
    }
}

void JsonHandler::GetOpenOrders(const boost::property_tree::ptree& pt, std::string& response) const {
    return _GetOpenOrders(
        pt, [](TradeRecord* trade) -> bool { return trade->cmd < OP_BUY || trade->cmd > OP_SELL; }, response);
}

void JsonHandler::GetPendingOrders(const boost::property_tree::ptree& pt, std::string& response) const {
    return _GetOpenOrders(
        pt, [](TradeRecord* trade) -> bool { return trade->cmd < OP_BUY_LIMIT || trade->cmd > OP_SELL_STOP; }, response);
}

void JsonHandler::GetClosedOrders(const boost::property_tree::ptree& pt, std::string& response) const {
    _GetClosedOrders(
        pt, [](TradeRecord* trade) -> bool { return trade->cmd < OP_BUY || trade->cmd > OP_SELL_STOP; }, response);
}

void JsonHandler::IsOpening(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    const ErrorCode* error_code = nullptr;
    std::string symbol = pt.get<std::string>("symbol", "");
    int time = pt.get<int>("time", 0);
    bool is_open = false;

    bool result = ServerApi::IsOpening(symbol.c_str(), time, &is_open, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    pt.put("is_open", is_open);
}

void JsonHandler::TradeTime(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    const ErrorCode* error_code = nullptr;
    time_t time = 0;

    bool result = ServerApi::CurrentTradeTime(&time, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    pt.put("trade_time", time);
}

void JsonHandler::GetSymbolList(ptree& pt) const {
    std::string request = pt.get<std::string>("request", "");
    const ErrorCode* error_code = nullptr;
    const ConSymbol* con_symbols = NULL;
    int total = 0;

    bool result = ServerApi::GetSymbolList(&total, &con_symbols, &error_code);

    pt.clear();
    SetResponseJson(pt, request, result, error_code);
    pt.put("count", total);
    if (result && con_symbols != NULL) {
        ptree symbols;
        for (int i = 0; i < total; i++) {
            ptree symbol;
            symbol.put("symbol", con_symbols[i].symbol);
            symbol.put("description", con_symbols[i].description);
            symbol.put("source", con_symbols[i].source);
            symbol.put("currency", con_symbols[i].currency);
            symbol.put("type", con_symbols[i].type);
            symbol.put("digits", con_symbols[i].digits);
            symbol.put("trade", con_symbols[i].trade);
            symbol.put("background_color", con_symbols[i].background_color);
            symbol.put("count", con_symbols[i].count);
            symbol.put("count_original", con_symbols[i].count_original);
            symbol.put("realtime", con_symbols[i].realtime);
            symbol.put("starting", con_symbols[i].starting);
            symbol.put("expiration", con_symbols[i].expiration);
            symbol.put("sessions", con_symbols[i].sessions);
            symbol.put("profit_mode", con_symbols[i].profit_mode);
            symbol.put("filter", con_symbols[i].filter);
            symbol.put("filter_counter", con_symbols[i].filter_counter);
            symbol.put("filter_limit", con_symbols[i].filter_limit);
            symbol.put("filter_smoothing", con_symbols[i].filter_smoothing);
            symbol.put("logging", con_symbols[i].logging);
            symbol.put("spread", con_symbols[i].spread);
            symbol.put("spread_balance", con_symbols[i].spread_balance);
            symbol.put("exemode", con_symbols[i].exemode);
            symbol.put("swap_enable", con_symbols[i].swap_enable);
            symbol.put("swap_type", con_symbols[i].swap_type);
            symbol.put("swap_long", con_symbols[i].swap_long);
            symbol.put("swap_short", con_symbols[i].swap_short);
            symbol.put("swap_rollover3days", con_symbols[i].swap_rollover3days);
            symbol.put("contract_size", con_symbols[i].contract_size);
            symbol.put("tick_value", con_symbols[i].tick_value);
            symbol.put("tick_size", con_symbols[i].tick_size);
            symbol.put("stops_level", con_symbols[i].stops_level);
            symbol.put("gtc_pendings", con_symbols[i].gtc_pendings);
            symbol.put("margin_mode", con_symbols[i].margin_mode);
            symbol.put("margin_initial", con_symbols[i].margin_initial);
            symbol.put("margin_maintenance", con_symbols[i].margin_maintenance);
            symbol.put("margin_hedged", con_symbols[i].margin_hedged);
            symbol.put("margin_divider", con_symbols[i].margin_divider);
            symbol.put("point", con_symbols[i].point);
            symbol.put("multiply", con_symbols[i].multiply);
            symbol.put("bid_tickvalue", con_symbols[i].bid_tickvalue);
            symbol.put("ask_tickvalue", con_symbols[i].ask_tickvalue);
            symbol.put("long_only", con_symbols[i].long_only);
            symbol.put("instant_max_volume", con_symbols[i].instant_max_volume);
            symbol.put("margin_currency", con_symbols[i].margin_currency);
            symbol.put("freeze_level", con_symbols[i].freeze_level);
            symbol.put("margin_hedged_strong", con_symbols[i].margin_hedged_strong);
            symbol.put("value_date", con_symbols[i].value_date);
            symbol.put("quotes_delay", con_symbols[i].quotes_delay);
            symbol.put("swap_openprice", con_symbols[i].swap_openprice);
            symbol.put("swap_variation_margin", con_symbols[i].swap_variation_margin);

            symbols.push_back(std::make_pair("", symbol));
        }
        pt.add_child("orders", symbols);
    }

    // no need free con_symbols
}

void JsonHandler::_GetOpenOrders(const ptree& pt, FilterOut filter_out, std::string& response) const {
    FUNC_WARDER;

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    const ErrorCode* error_code = nullptr;
    TradeRecord* trade_record = NULL;
    int total = 0;
    int count = 0;

    bool result = ServerApi::GetOpenOrders(login, &total, &trade_record, &error_code);

    try {
        // reduce re-allocate memory
        response.reserve(total * 500 + 128);
        response.append("{");
        response.append("\"request\":\"").append(request).append("\",");
        response.append("\"result\":\"").append(result ? "OK" : "ERROR").append("\",");
        response.append("\"error_code\":").append(std::to_string(error_code->m_code)).append(",");
        response.append("\"error_des\":\"").append(error_code->m_des).append("\",");
        response.append("\"orders\":").append("[");
        if (result && trade_record != NULL) {
            for (int i = 0; i < total; i++) {
                TradeRecord* trade = trade_record + i;
                if (filter_out(trade)) {
                    continue;
                }
                count++;
                AppendTradeRecordJsonStr(trade, response);
                response.append(",");
            }
            if (count > 0) {
                response.pop_back();
            }
        }
        response.append("],");
        response.append("\"count\":").append(std::to_string(count));
        response.append("}\r\n");
    } catch (...) {
        LOG("No memory to perform getting open orders");
        response.clear();
        response.append("{");
        response.append("\"request\":\"").append(request).append("\",");
        response.append("\"result\":\"").append(ErrorCode::EC_NO_MEMORY.m_code == 0 ? "OK" : "ERROR").append("\",");
        response.append("\"error_code\":").append(std::to_string(ErrorCode::EC_NO_MEMORY.m_code)).append(",");
        response.append("\"total\":").append(std::to_string(total)).append(",");
        response.append("\"error_des\":\"").append(ErrorCode::EC_NO_MEMORY.m_des).append("\"}\r\n");
    }

    if (trade_record != NULL) {
        HEAP_FREE(trade_record);
        trade_record = NULL;
    }
}

void JsonHandler::_GetClosedOrders(const ptree& pt, FilterOut filter_out, std::string& response) const {
    FUNC_WARDER;

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int from = pt.get<int>("from", -1);
    int to = pt.get<int>("to", -1);
    const ErrorCode* error_code = nullptr;
    TradeRecord* trade_record = NULL;
    int total = 0;
    int count = 0;

    bool result = ServerApi::GetClosedOrders(login, from, to, &total, &trade_record, &error_code);

    try {
        LOG("Closed orders: %d", total);
        response.reserve(total * 500 + 128);
        response.append("{");
        response.append("\"request\":\"").append(request).append("\",");
        response.append("\"result\":\"").append(result ? "OK" : "ERROR").append("\",");
        response.append("\"error_code\":").append(std::to_string(error_code->m_code)).append(",");
        response.append("\"error_des\":\"").append(error_code->m_des).append("\",");
        response.append("\"orders\":").append("[");
        if (result && trade_record != NULL) {
            for (int i = 0; i < total; i++) {
                ptree order;
                TradeRecord* trade = trade_record + i;
                if (filter_out(trade)) {
                    continue;
                }
                count++;
                AppendTradeRecordJsonStr(trade, response);
                response.append(",");
            }
            if (count > 0) {
                response.pop_back();
            }
        }
        response.append("],");
        response.append("\"count\":").append(std::to_string(count));
        response.append("}\r\n");
    } catch (...) {
        LOG("No memory to perform getting closed orders");
        response.clear();
        response.append("{");
        response.append("\"request\":\"").append(request).append("\",");
        response.append("\"result\":\"").append(ErrorCode::EC_NO_MEMORY.m_code == 0 ? "OK" : "ERROR").append("\",");
        response.append("\"error_code\":").append(std::to_string(ErrorCode::EC_NO_MEMORY.m_code)).append(",");
        response.append("\"total\":").append(std::to_string(total)).append(",");
        response.append("\"error_des\":\"").append(ErrorCode::EC_NO_MEMORY.m_des).append("\"}\r\n");
    }

    if (trade_record != NULL) {
        HEAP_FREE(trade_record);
        trade_record = NULL;
    }
}

void JsonHandler::AppendTradeRecordJsonStr(TradeRecord* trade, std::string& response) const {
    response.append("{");
    response.append("\"order\":").append(std::to_string(trade->order)).append(",");
    response.append("\"login\":").append(std::to_string(trade->login)).append(",");
    response.append("\"symbol\":\"").append(trade->symbol).append("\",");
    response.append("\"digits\":").append(std::to_string(trade->digits)).append(",");
    response.append("\"cmd\":\"").append(TradeCmdStr(trade->cmd)).append("\",");
    response.append("\"volume\":").append(std::to_string(trade->volume)).append(",");
    response.append("\"open_time\":").append(std::to_string(trade->open_time)).append(",");
    response.append("\"state\":\"").append(ToTradeRecordStateStr(trade->state)).append("\",");
    response.append("\"open_price\":").append(std::to_string(trade->open_price)).append(",");
    response.append("\"sl\":").append(std::to_string(trade->sl)).append(",");
    response.append("\"tp\":").append(std::to_string(trade->tp)).append(",");
    response.append("\"close_time\":").append(std::to_string(trade->close_time)).append(",");
    response.append("\"gw_volume\":").append(std::to_string(trade->gw_volume)).append(",");
    response.append("\"expiration\":").append(std::to_string(trade->expiration)).append(",");
    response.append("\"commission\":").append(std::to_string(trade->commission)).append(",");
    response.append("\"commission_agent\":").append(std::to_string(trade->commission_agent)).append(",");
    response.append("\"storage\":").append(std::to_string(trade->storage)).append(",");
    response.append("\"close_price\":").append(std::to_string(trade->close_price)).append(",");
    response.append("\"profit\":").append(std::to_string(NormalizeDouble(trade->profit, 2))).append(",");
    response.append("\"taxes\":").append(std::to_string(trade->taxes)).append(",");
    response.append("\"magic\":").append(std::to_string(trade->magic)).append(",");
    response.append("\"comment\":\"").append(trade->comment).append("\",");
    response.append("\"gw_order\":").append(std::to_string(trade->gw_order)).append(",");
    response.append("\"activation\":").append(std::to_string(trade->activation)).append(",");
    response.append("\"gw_open_price\":").append(std::to_string(trade->gw_open_price)).append(",");
    response.append("\"gw_close_price\":").append(std::to_string(trade->gw_close_price)).append(",");
    response.append("\"margin_rate\":").append(std::to_string(trade->margin_rate)).append(",");
    response.append("\"timestamp\":").append(std::to_string(trade->timestamp));
    response.append("}");
}

void JsonHandler::SetResponseJson(boost::property_tree::ptree& response, const std::string& request, bool result, const ErrorCode* error_code) const {
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
}
