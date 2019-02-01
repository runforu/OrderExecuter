
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "ErrorCode.h"
#include "JsonWrapper.h"
#include "Loger.h"
#include "MiscJsonHandler.h"
#include "common.h"
#include "ManagerApi.h"

using namespace boost::property_tree;
using namespace http::server;

int MiscJsonHandler::get_priority() const {
    return 99;
}

bool MiscJsonHandler::can_handle(const request& req) {
    return std::find_if(req.headers.begin(), req.headers.end(), [&](const http::server::header& h) {
               return h == http::server::header::json_content_type;
           }) != req.headers.end();
}

bool MiscJsonHandler::handle(const http::server::request& req, http::server::reply& rep) {
    ptree pt = JsonWrapper::ParseJson(req.body);
    LOG("MiscJsonHandler -> %s", req.body.c_str());

    ptree response;
    if (!pt.empty()) {
        rep.status = reply::ok;
        if (pt.get<std::string>("request", "").compare("RequestChart") == 0) {
            response = RequestChart(pt);
        } else {
            return false;
        }
    } else {
        // Handle json parse error
        rep.status = reply::bad_request;
        response.put("json_error", "Invalid json format");
    }

    rep.headers.push_back(header::json_content_type);
    std::string content = JsonWrapper::ToJsonStr(response);
    rep.headers.push_back(header("Content-Length", std::to_string(content.length())));
    rep.content.append(content);
    return true;
}

ptree MiscJsonHandler::RequestChart(ptree pt) {
    std::string request = pt.get<std::string>("request", "");
    std::string symbol = pt.get<std::string>("symbol", "");
    int period = pt.get<int>("period", 30);
    std::string mode_str = pt.get<std::string>("mode", "CHART_RANGE_IN");
    __time32_t start = pt.get<__time32_t>("start", -1);
    __time32_t end = pt.get<__time32_t>("end", -1);
    __time32_t timestamp = pt.get<__time32_t>("timestamp", -1);

    int mode = CHART_RANGE_IN;
    if (mode_str.compare("CHART_RANGE_OUT") == 0) {
        mode = CHART_RANGE_OUT;
    } else if (mode_str.compare("CHART_RANGE_LAST") == 0) {
        mode = CHART_RANGE_LAST;
    }

    ptree response;
    const ErrorCode* error_code =
        ManagerApi::Instance().RequestChart(symbol.c_str(), period, mode, start, end, timestamp, response);
    response.put("result", error_code->m_code == 0 ? "OK" : "ERROR");
    response.put("error_code", error_code->m_code);
    response.put("error_des", error_code->m_des);
    return response;
}
