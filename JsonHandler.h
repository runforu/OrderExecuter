#ifndef _JSON_HANDLER_H_
#define _JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "http_server/request_handler.h"

struct TradeRecord;

class JsonHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;
    virtual bool can_handle(const http::server::request& req);
    virtual bool handle(const http::server::request& req, http::server::reply& rep);
    virtual ~JsonHandler() {}

private:
    typedef bool (*FilterOut)(TradeRecord*);
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
    boost::property_tree::ptree AddUser(boost::property_tree::ptree pt);
    inline void GetOpenOrders(boost::property_tree::ptree pt, std::string& response);
    inline void GetPendingOrders(boost::property_tree::ptree pt, std::string& response);
    inline void GetClosedOrders(boost::property_tree::ptree pt, std::string& response);
    inline boost::property_tree::ptree IsOpening(boost::property_tree::ptree pt);
    inline boost::property_tree::ptree TradeTime(boost::property_tree::ptree pt);
    inline boost::property_tree::ptree GetSymbolList(boost::property_tree::ptree pt);

private:
    void _GetOpenOrders(boost::property_tree::ptree pt, FilterOut filter, std::string& response);
    void _GetClosedOrders(boost::property_tree::ptree pt, FilterOut filter, std::string& response);
    void AppendTradeRecordJsonStr(TradeRecord* trade_record, std::string& response);
};

#endif  // !_JSON_HANDLER_H_