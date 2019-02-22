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

    if (!pt.empty()) {
        rep.status = reply::ok;
        if (pt.get<std::string>("request", "").compare("RequestChart") == 0) {
            RequestChart(pt, rep.content);
        } else {
            return false;
        }
    } else {
        // Handle json parse error
        rep.status = reply::bad_request;
        rep.content.append("{\"json_error\":\"Invalid json format\"}");
    }

    rep.headers.push_back(header::json_content_type);
    rep.headers.push_back(header("Content-Length", std::to_string(rep.content.length())));
    return true;
}

void MiscJsonHandler::RequestChart(ptree pt, std::string& content) {
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

    char* json_str = NULL;
    const ErrorCode* error_code;
    int total = 0;
    RateInfo* rate_info =
        ManagerApi::Instance().RequestChart(symbol.c_str(), period, mode, start, end, &timestamp, &total, &error_code);
    if (rate_info == NULL) {
        content.reserve(256);
        content.append("{");
        content.append("\"result\":\"").append(error_code->m_code == 0 ? "OK" : "ERROR").append("\",");
        content.append("\"error_code\":").append(std::to_string(error_code->m_code)).append(",");
        content.append("\"error_des\":\"").append(error_code->m_des).append("\"");
        content.append("}");
    } else {
        // max length of a RateInfo json is less than 128, the other info is less than 256
        content.reserve(total * 128 + 256);
        content.append("{");
        content.append("\"result\":\"").append(error_code->m_code == 0 ? "OK" : "ERROR").append("\",");
        content.append("\"error_code\":").append(std::to_string(error_code->m_code)).append(",");
        content.append("\"error_des\":\"").append(error_code->m_des).append("\",");
        content.append("\"count\":").append(std::to_string(total)).append(",");
        content.append("\"timesign\":").append(std::to_string(timestamp)).append(",");
        content.append("\"rate_infos\":").append("[");
        for (int i = 0; i < total; i++) {
            content.append("{");
            content.append("\"open_price\":").append(std::to_string(rate_info[i].open)).append(",");
            content.append("\"high\":").append(std::to_string(rate_info[i].high)).append(",");
            content.append("\"low\":").append(std::to_string(rate_info[i].low)).append(",");
            content.append("\"close\":").append(std::to_string(rate_info[i].close)).append(",");
            content.append("\"time\":").append(std::to_string(rate_info[i].ctm)).append(",");
            content.append("\"volume\":").append(std::to_string(rate_info[i].vol));
            content.append("}");
            if (i != total - 1) {
                content.append(",");
            }
        }
        content.append("]}");
        free(rate_info);
    }
}
