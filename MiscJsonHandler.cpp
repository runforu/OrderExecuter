#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "ErrorCode.h"
#include "JsonWrapper.h"
#include "Loger.h"
#include "ManagerApi.h"
#include "MiscJsonHandler.h"
#include "common.h"

using namespace boost::property_tree;

int MiscJsonHandler::get_priority() const {
    return 99;
}

bool MiscJsonHandler::can_handle(const http::server::request& req) const {
    return std::any_of(req.headers.begin(), req.headers.end(),
                       [](const http::server::header& h) { return h.name == "content-type" && h.value == "application/json"; });
}

bool MiscJsonHandler::handle(const http::server::request& req, http::server::reply& rep) const {
    ptree pt;
    JsonWrapper::ParseJson(req.body, pt);

    if (!pt.empty()) {
        rep.status = http::server::reply::ok;
        if (pt.get<std::string>("request", "").compare("RequestChart") == 0) {
            RequestChart(pt, rep.content);
        } else {
            return false;
        }
    } else {
        // Handle json parse error
        rep.status = http::server::reply::bad_request;
        rep.content.append("{\"json_error\":\"Invalid json format\"}");
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

void MiscJsonHandler::RequestChart(ptree pt, std::string& content) const {
    std::string request = pt.get<std::string>("request", "");
    std::string symbol = pt.get<std::string>("symbol", "");
    int login = pt.get<int>("login", -1);
    int period = pt.get<int>("period", 30);
    std::string mode_str = pt.get<std::string>("mode", "CHART_RANGE_IN");
    __time32_t start = pt.get<__time32_t>("start", 0);
    __time32_t end = pt.get<__time32_t>("end", 0);
    __time32_t timestamp = pt.get<__time32_t>("timestamp", 0);

    ManagerApi::Instance().RequestChart(login, symbol.c_str(), period, mode_str, start, end, &timestamp, content);
}
