
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "json_handler.h"
#include "json_wrapper.h"

namespace http {
namespace server {

int json_handler::get_priority() const {
    return 100;
}

bool json_handler::handle(const request& req, reply& rep) {
    if (std::find_if(req.headers.begin(), req.headers.end(), [&](const header& h) { return h == header::content_type; }) !=
        req.headers.end()) {
        ptree pt = json_wrapper::parse_json(req.body);
        if (pt.get<std::string>("request", "").compare("OpenOrder") == 0) {
            OpenOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("AddOrder") == 0) {
            AddOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("UpdateOrder") == 0) {
            UpdateOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("CloseOrder") == 0) {
            CloseOrder(pt);
        } else if (pt.get<std::string>("request", "").compare("Deposit") == 0) {
            Deposit(pt);
        } else if (pt.get<std::string>("request", "").compare("GetUserRecord") == 0) {
            GetUserRecord(pt);
        } else if (pt.get<std::string>("request", "").compare("UpdateUserRecord") == 0) {
            UpdateUserRecord(pt);
        } else {
            return false;
        }
        rep.headers.push_back(header::content_type);
        std::string content = "{\"result\":\"ok\"}\n";
        rep.headers.push_back(header("Content-Length", std::to_string(content.length())));
        rep.content.append(content);
        return true;
    }

    return false;
}

ptree json_handler::OpenOrder(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<int>("login", -1) << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    std::cout << pt.get<std::string>("symbol", "") << std::endl;
    std::cout << pt.get<std::string>("cmd", "") << std::endl;
    std::cout << pt.get<int>("volume", 0) << std::endl;
    std::cout << pt.get<double>("open_price", 0.0) << std::endl;
    std::cout << pt.get<double>("sl", 0.0) << std::endl;
    std::cout << pt.get<double>("tp", 0.0) << std::endl;
    std::cout << pt.get<std::string>("comment", "") << std::endl;
    return pt;
}

ptree json_handler::AddOrder(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<int>("login", -1) << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    std::cout << pt.get<std::string>("symbol", "") << std::endl;
    std::cout << pt.get<std::string>("cmd", "") << std::endl;
    std::cout << pt.get<int>("volume", 0) << std::endl;
    std::cout << pt.get<double>("open_price", 0.0) << std::endl;
    std::cout << pt.get<double>("sl", 0.0) << std::endl;
    std::cout << pt.get<double>("tp", 0.0) << std::endl;
    std::cout << pt.get<std::string>("comment", "") << std::endl;
    return pt;
}

ptree json_handler::UpdateOrder(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    std::cout << pt.get<int>("order", 0) << std::endl;
    std::cout << pt.get<double>("open_price", 0.0) << std::endl;
    std::cout << pt.get<double>("sl", 0.0) << std::endl;
    std::cout << pt.get<double>("tp", 0.0) << std::endl;
    std::cout << pt.get<std::string>("comment", "") << std::endl;
    return pt;
}

ptree json_handler::CloseOrder(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    std::cout << pt.get<int>("order", 0) << std::endl;
    std::cout << pt.get<double>("close_price", 0.0) << std::endl;
    std::cout << pt.get<std::string>("comment", "") << std::endl;
    return pt;
}

ptree json_handler::Deposit(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<int>("login", -1) << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    std::cout << pt.get<double>("balance", 0.0) << std::endl;
    std::cout << pt.get<std::string>("comment", "") << std::endl;
    return pt;
}

ptree json_handler::GetUserRecord(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<int>("login", -1) << std::endl;
    std::cout << pt.get<int>("user", -1) << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    return pt;
}

ptree json_handler::UpdateUserRecord(ptree pt) {
    std::cout << pt.get<std::string>("request", "") << std::endl;
    std::cout << pt.get<int>("login", -1) << std::endl;
    std::cout << pt.get<int>("user", -1) << std::endl;
    std::cout << pt.get<std::string>("group", "") << std::endl;
    std::cout << pt.get<std::string>("ip", "0.0.0.0") << std::endl;
    std::cout << pt.get<std::string>("name", "") << std::endl;
    std::cout << pt.get<std::string>("phone", "") << std::endl;
    std::cout << pt.get<std::string>("email", "") << std::endl;
    std::cout << pt.get<std::string>("password", "") << std::endl;
    std::cout << pt.get<int>("enable", -1) << std::endl;
    std::cout << pt.get<int>("leverage", 0) << std::endl;    
    return pt;
}

}  // namespace server
}  // namespace http
