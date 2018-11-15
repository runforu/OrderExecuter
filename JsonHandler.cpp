
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "Loger.h"
#include "ServerApi.h"
#include "common.h"
#include "JsonHandler.h"
#include "JsonWrapper.h"

using namespace boost::property_tree;
using namespace http::server;

int JsonHandler::get_priority() const {
    return 100;
}

bool JsonHandler::handle(const http::server::request& req, http::server::reply& rep) {
    if (std::find_if(req.headers.begin(), req.headers.end(), [&](const http::server::header& h) {
            return h == http::server::header::content_type;
        }) != req.headers.end()) {
        ptree pt = JsonWrapper::ParseJson(req.body);
        ptree response;
        if (pt.get<std::string>("request", "").compare("OpenOrder") == 0) {
            response = OpenOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("AddOrder") == 0) {
            response = AddOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("UpdateOrder") == 0) {
            response = UpdateOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("CloseOrder") == 0) {
            response = CloseOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("Deposit") == 0) {
            response = Deposit(pt);
        } else if (pt.get<std::string>("request", "").compare("GetUserRecord") == 0) {
            response = GetUserRecord(pt);
        } else if (pt.get<std::string>("request", "").compare("UpdateUserRecord") == 0) {
            response = UpdateUserRecord(pt);
        } else if (pt.get<std::string>("request", "").compare("ChangePassword") == 0) {
            response = ChangePassword(pt);
        } else if (pt.get<std::string>("request", "").compare("CheckPassword") == 0) {
            response = CheckPassword(pt);
        } else if (pt.get<std::string>("request", "").compare("GetMargin") == 0) {
            response = GetMargin(pt);
        } else if (pt.get<std::string>("request", "").compare("GetOrder") == 0) {
            response = GetOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("GetOpenOrders") == 0) {
            response = GetOpenOrders(pt);
        } else if (pt.get<std::string>("request", "").compare("GetPendingOrders") == 0) {
            response = GetPendingOrders(pt);
        } else if (pt.get<std::string>("request", "").compare("GetClosedOrders") == 0) {
            response = GetClosedOrders(pt);
        } else {
            return false;
        }
        rep.status = reply::ok;
        rep.headers.push_back(header::content_type);
        std::string content = JsonWrapper::ToJsonStr(response);
        rep.headers.push_back(header("Content-Length", std::to_string(content.length())));
        rep.content.append(content);
        return true;
    }

    return false;
}

ptree JsonHandler::OpenOrder(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<std::string>("symbol", ""));
    LOG(pt.get<std::string>("cmd", ""));
    LOG(pt.get<int>("volume", 0));
    LOG(pt.get<double>("open_price", 0.0));
    LOG(pt.get<double>("sl", 0.0));
    LOG(pt.get<double>("tp", 0.0));
    LOG(pt.get<std::string>("comment", ""));

    const ErrorCode* error_code;
    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    std::string symbol = pt.get<std::string>("symbol", "");
    int cmd = ToCmd(pt.get<std::string>("cmd", ""));
    int volume = pt.get<int>("volume", 0);
    double open_price = pt.get<double>("open_price", 0.0);
    double sl = pt.get<double>("sl", 0.0);
    double tp = pt.get<double>("tp", 0.0);
    std::string comment = pt.get<std::string>("comment", "");
    int order;

    bool result = ServerApi::OpenOrder(login, ip.c_str(), symbol.c_str(), cmd, volume, open_price, sl, tp, comment.c_str(),
                                       &error_code, &order);
    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result) {
        response.put("order", order);
    }
    return response;
}

ptree JsonHandler::AddOrder(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<std::string>("symbol", ""));
    LOG(pt.get<std::string>("cmd", ""));
    LOG(pt.get<int>("volume", 0));
    LOG(pt.get<double>("open_price", 0.0));
    LOG(pt.get<double>("sl", 0.0));
    LOG(pt.get<double>("tp", 0.0));
    LOG(pt.get<std::string>("comment", ""));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    std::string symbol = pt.get<std::string>("symbol", "");
    int cmd = ToCmd(pt.get<std::string>("cmd", ""));
    int volume = pt.get<int>("volume", 0);
    double open_price = pt.get<double>("open_price", 0.0);
    double sl = pt.get<double>("sl", 0.0);
    double tp = pt.get<double>("tp", 0.0);
    std::string comment = pt.get<std::string>("comment", "");
    int order;
    const ErrorCode* error_code;

    bool result = ServerApi::AddOrder(login, ip.c_str(), symbol.c_str(), cmd, volume, open_price, sl, tp, comment.c_str(),
                                      &error_code, &order);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result) {
        response.put("order", order);
    }
    return response;
}

ptree JsonHandler::UpdateOrder(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<int>("order", 0));
    LOG(pt.get<double>("open_price", 0.0));
    LOG(pt.get<double>("sl", 0.0));
    LOG(pt.get<double>("tp", 0.0));
    LOG(pt.get<std::string>("comment", ""));

    std::string request = pt.get<std::string>("request", "");
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    int order = pt.get<int>("order", 0);
    double open_price = pt.get<double>("open_price", 0.0);
    double sl = pt.get<double>("sl", 0.0);
    double tp = pt.get<double>("tp", 0.0);
    std::string comment = pt.get<std::string>("comment", "");
    const ErrorCode* error_code;

    bool result = ServerApi::UpdateOrder(ip.c_str(), order, open_price, sl, tp, comment.c_str(), &error_code);
    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);

    return response;
}

ptree JsonHandler::CloseOrder(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<int>("order", 0));
    LOG(pt.get<double>("close_price", 0.0));
    LOG(pt.get<std::string>("comment", ""));

    std::string request = pt.get<std::string>("request", "");
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    int order = pt.get<int>("order", 0);
    double close_price = pt.get<double>("close_price", 0.0);
    std::string comment = pt.get<std::string>("comment", "");
    const ErrorCode* error_code;

    bool result = ServerApi::CloseOrder(ip.c_str(), order, close_price, comment.c_str(), &error_code);
    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);

    return response;
}

ptree JsonHandler::Deposit(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<double>("balance", 0.0));
    LOG(pt.get<std::string>("comment", ""));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    double value = pt.get<double>("value", 0.0);
    std::string comment = pt.get<std::string>("comment", "");
    const ErrorCode* error_code;
    double order = 0;
    bool result = ServerApi::Deposit(login, ip.c_str(), value, comment.c_str(), &order, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    if (result) {
        response.put("order", order);
    }
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);

    return response;
}

ptree JsonHandler::GetUserRecord(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<int>("user", -1));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int user = pt.get<int>("user", -1);
    std::string ip = pt.get<std::string>("ip", "0.0.0.0");
    const ErrorCode* error_code;

    UserRecord user_record;
    bool result = ServerApi::GetUserRecord(user, &user_record, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result) {
        response.put("login", user_record.login);
        response.put("group", user_record.group);
        response.put("password", user_record.password);
        response.put("enable", user_record.enable);
        response.put("enable_change_password", user_record.enable_change_password);
        response.put("enable_read_only", user_record.enable_read_only);
        response.put("enable_otp", user_record.enable_otp);
        response.put("password_investor", user_record.password_investor);
        response.put("password_phone", user_record.password_phone);
        response.put("name", user_record.name);
        response.put("country", user_record.country);
        response.put("city", user_record.city);
        response.put("state", user_record.state);
        response.put("zipcode", user_record.zipcode);
        response.put("address", user_record.address);
        response.put("lead_source", user_record.lead_source);
        response.put("phone", user_record.phone);
        response.put("email", user_record.email);
        response.put("comment", user_record.comment);
        response.put("id", user_record.id);
        response.put("status", user_record.status);
        response.put("regdate", user_record.regdate);
        response.put("lastdate", user_record.lastdate);
        response.put("leverage", user_record.leverage);
        response.put("agent_account", user_record.agent_account);
        response.put("timestamp", user_record.timestamp);
        response.put("last_ip", user_record.last_ip);
        response.put("balance", user_record.balance);
        response.put("prevmonthbalance", user_record.prevmonthbalance);
        response.put("prevbalance", user_record.prevbalance);
        response.put("credit", user_record.credit);
        response.put("interestrate", user_record.interestrate);
        response.put("taxes", user_record.taxes);
        response.put("prevmonthequity", user_record.prevmonthequity);
        response.put("prevequity", user_record.prevequity);
        response.put("otp_secret", user_record.otp_secret);
        response.put("send_reports", user_record.send_reports);
        response.put("mqid", user_record.mqid);
        response.put("user_color", user_record.user_color);
    }

    return response;
}

ptree JsonHandler::UpdateUserRecord(ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<int>("user", -1));
    LOG(pt.get<std::string>("group", ""));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<std::string>("name", ""));
    LOG(pt.get<std::string>("phone", ""));
    LOG(pt.get<std::string>("email", ""));
    LOG(pt.get<int>("enable", -1));
    LOG(pt.get<int>("leverage", -1));

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
    const ErrorCode* error_code;

    bool result =
        ServerApi::UpdateUserRecord(user, group.c_str(), name.c_str(), phone.c_str(), email.c_str(), enable, leverage, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);

    return response;
}

boost::property_tree::ptree JsonHandler::ChangePassword(boost::property_tree::ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<std::string>("password", ""));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string password = pt.get<std::string>("password", "");
    const ErrorCode* error_code;

    bool result = ServerApi::ChangePassword(login, password.c_str(), &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);

    return response;
}

boost::property_tree::ptree JsonHandler::CheckPassword(boost::property_tree::ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<std::string>("ip", "0.0.0.0"));
    LOG(pt.get<std::string>("password", ""));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    std::string password = pt.get<std::string>("password", "");
    const ErrorCode* error_code;

    bool result = ServerApi::CheckPassword(login, password.c_str(), &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);

    return response;
}

boost::property_tree::ptree JsonHandler::GetMargin(boost::property_tree::ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    const ErrorCode* error_code;
    UserInfo user_info = {0};
    double margin;
    double freemargin;
    double equity;

    bool result = ServerApi::GetMargin(login, &user_info, &margin, &freemargin, &equity, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result) {
        response.put("margin", margin);
        response.put("freemargin", freemargin);
        response.put("equity", equity);
        response.put("group", user_info.group);
        response.put("ip", user_info.ip);
        response.put("leverage", user_info.leverage);
        response.put("balance", user_info.balance);
        response.put("credit", user_info.credit);
    }

    return response;
}

boost::property_tree::ptree JsonHandler::GetOrder(boost::property_tree::ptree pt) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<int>("order", -1));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int order = pt.get<int>("order", -1);
    const ErrorCode* error_code;
    TradeRecord trade_record = {0};

    bool result = ServerApi::GetOrder(order, &trade_record, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result) {
        response.put("order", trade_record.order);
        response.put("login", trade_record.login);
        response.put("symbol", trade_record.symbol);
        response.put("digits", trade_record.digits);
        response.put("cmd", trade_record.cmd);
        response.put("volume", trade_record.volume);
        response.put("open_time", trade_record.open_time);
        response.put("state", ToTradeRecordStateStr(trade_record.state));
        response.put("open_price", trade_record.open_price);
        response.put("sl", trade_record.sl);
        response.put("tp", trade_record.tp);
        response.put("close_time", trade_record.close_time);
        response.put("gw_volume", trade_record.gw_volume);
        response.put("expiration", trade_record.expiration);
        response.put("commission", trade_record.commission);
        response.put("commission_agent", trade_record.commission_agent);
        response.put("storage", trade_record.storage);
        response.put("close_price", trade_record.close_price);
        response.put("profit", NormalizeDouble(trade_record.profit, 2));
        response.put("taxes", trade_record.taxes);
        response.put("magic", trade_record.magic);
        response.put("comment", trade_record.comment);
        response.put("gw_order", trade_record.gw_order);
        response.put("activation", trade_record.activation);
        response.put("gw_open_price", trade_record.gw_open_price);
        response.put("gw_close_price", trade_record.gw_close_price);
        response.put("margin_rate", trade_record.margin_rate);
        response.put("timestamp", trade_record.timestamp);
    }

    return response;
}

boost::property_tree::ptree JsonHandler::GetOpenOrders(boost::property_tree::ptree pt) {
    return _GetOpenOrders(pt, [](TradeRecord& trade) -> bool { return trade.cmd < OP_BUY || trade.cmd > OP_SELL; });
}

boost::property_tree::ptree JsonHandler::GetPendingOrders(boost::property_tree::ptree pt) {
    return _GetOpenOrders(pt, [](TradeRecord& trade) -> bool { return trade.cmd < OP_BUY_LIMIT || trade.cmd > OP_SELL_STOP; });
}

boost::property_tree::ptree JsonHandler::GetClosedOrders(boost::property_tree::ptree pt) {
    return _GetClosedOrders(pt,
                            [](TradeRecord& trade) -> bool { return false && trade.cmd < OP_BUY || trade.cmd > OP_SELL_STOP; });
}

boost::property_tree::ptree JsonHandler::_GetOpenOrders(boost::property_tree::ptree pt, FilterOut filter_out) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    const ErrorCode* error_code;
    TradeRecord* trade_record = NULL;
    int total = 0;
    int count = 0;

    bool result = ServerApi::GetOpenOrders(login, &total, &trade_record, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result && trade_record != NULL) {
        ptree orders;
        for (int i = 0; i < total; i++) {
            ptree order;
            TradeRecord trade = trade_record[i];
            if (filter_out(trade)) {
                continue;
            }
            count++;
            order.put("order", trade.order);
            order.put("login", trade.login);
            order.put("symbol", trade.symbol);
            order.put("digits", trade.digits);
            order.put("cmd", TradeCmdStr(trade.cmd));
            order.put("volume", trade.volume);
            order.put("open_time", trade.open_time);
            order.put("state", ToTradeRecordStateStr(trade.state));
            order.put("open_price", trade.open_price);
            order.put("sl", trade.sl);
            order.put("tp", trade.tp);
            order.put("close_time", trade.close_time);
            order.put("gw_volume", trade.gw_volume);
            order.put("expiration", trade.expiration);
            order.put("commission", trade.commission);
            order.put("commission_agent", trade.commission_agent);
            order.put("storage", trade.storage);
            order.put("close_price", trade.close_price);
            order.put("profit", NormalizeDouble(trade.profit, 2));
            order.put("taxes", trade.taxes);
            order.put("magic", trade.magic);
            order.put("comment", trade.comment);
            order.put("gw_order", trade.gw_order);
            order.put("activation", trade.activation);
            order.put("gw_open_price", trade.gw_open_price);
            order.put("gw_close_price", trade.gw_close_price);
            order.put("margin_rate", trade.margin_rate);
            order.put("timestamp", trade.timestamp);
            orders.push_back(std::make_pair("", order));
        }
        response.put("count", count);
        response.add_child("orders", orders);
    }

    if (trade_record != NULL) {
        HEAP_FREE(trade_record);
        trade_record = NULL;
    }

    return response;
}

boost::property_tree::ptree JsonHandler::_GetClosedOrders(boost::property_tree::ptree pt, FilterOut filter_out) {
    LOG(pt.get<std::string>("request", ""));
    LOG(pt.get<int>("login", -1));
    LOG(pt.get<int>("from", -1));
    LOG(pt.get<int>("to", -1));

    std::string request = pt.get<std::string>("request", "");
    int login = pt.get<int>("login", -1);
    int from = pt.get<int>("from", -1);
    int to = pt.get<int>("to", -1);
    const ErrorCode* error_code;
    TradeRecord* trade_record;
    int total = 0;
    int count = 0;

    bool result = ServerApi::GetClosedOrders(login, from, to, &total, &trade_record, &error_code);

    ptree response;
    response.put("request", request);
    response.put("result", result ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    if (result && trade_record != NULL) {
        ptree orders;
        for (int i = 0; i < total; i++) {
            ptree order;
            TradeRecord trade = trade_record[i];
            if (filter_out(trade)) {
                continue;
            }
            count++;
            order.put("order", trade.order);
            order.put("login", trade.login);
            order.put("symbol", trade.symbol);
            order.put("digits", trade.digits);
            order.put("cmd", TradeCmdStr(trade.cmd));
            order.put("volume", trade.volume);
            order.put("open_time", trade.open_time);
            order.put("state", ToTradeRecordStateStr(trade.state));
            order.put("open_price", trade.open_price);
            order.put("sl", trade.sl);
            order.put("tp", trade.tp);
            order.put("close_time", trade.close_time);
            order.put("gw_volume", trade.gw_volume);
            order.put("expiration", trade.expiration);
            order.put("commission", trade.commission);
            order.put("commission_agent", trade.commission_agent);
            order.put("storage", trade.storage);
            order.put("close_price", trade.close_price);
            order.put("profit", NormalizeDouble(trade.profit, 2));
            order.put("taxes", trade.taxes);
            order.put("magic", trade.magic);
            order.put("comment", trade.comment);
            order.put("gw_order", trade.gw_order);
            order.put("activation", trade.activation);
            order.put("gw_open_price", trade.gw_open_price);
            order.put("gw_close_price", trade.gw_close_price);
            order.put("margin_rate", trade.margin_rate);
            order.put("timestamp", trade.timestamp);
            orders.push_back(std::make_pair("", order));
        }
        response.put("count", count);
        response.add_child("orders", orders);
    }

    if (trade_record != NULL) {
        HEAP_FREE(trade_record);
        trade_record = NULL;
    }

    return response;
}
