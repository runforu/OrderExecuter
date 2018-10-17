#ifndef _HTTP_SERVER_JSON_HANDLER_H_
#define _HTTP_SERVER_JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "http_server/request_handler.h"
#include "../include/MT4ServerAPI.h"

class JsonHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;
    virtual bool handle(const http::server::request& req, http::server::reply& rep);
    virtual ~JsonHandler() {}
    JsonHandler() {}

private:
    typedef bool (*FilterOut)(TradeRecord&);
    boost::property_tree::ptree OpenOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree AddOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree UpdateOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree CloseOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree Deposit(boost::property_tree::ptree pt);
    boost::property_tree::ptree GetUserRecord(boost::property_tree::ptree pt);
    boost::property_tree::ptree UpdateUserRecord(boost::property_tree::ptree pt);
    boost::property_tree::ptree ChangePassword(boost::property_tree::ptree pt);
    boost::property_tree::ptree CheckPassword(boost::property_tree::ptree pt);
    boost::property_tree::ptree GetMargin(boost::property_tree::ptree pt);
    boost::property_tree::ptree GetOrder(boost::property_tree::ptree pt);
    inline boost::property_tree::ptree GetOpenOrders(boost::property_tree::ptree pt);
    inline boost::property_tree::ptree GetPendingOrders(boost::property_tree::ptree pt);
    inline boost::property_tree::ptree GetClosedOrders(boost::property_tree::ptree pt);

private:
    boost::property_tree::ptree _GetOpenOrders(boost::property_tree::ptree pt, FilterOut filter);
    boost::property_tree::ptree _GetClosedOrders(boost::property_tree::ptree pt, FilterOut filter);
};

#endif  // !_HTTP_SERVER_JSON_HANDLER_H_