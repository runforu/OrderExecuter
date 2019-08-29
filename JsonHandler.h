#ifndef _JSON_HANDLER_H_
#define _JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "http_server/request_handler.h"

struct TradeRecord;
struct ErrorCode;

class JsonHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;
    virtual bool can_handle(const http::server::request& req);
    virtual bool handle(const http::server::request& req, http::server::reply& rep);
    virtual ~JsonHandler() {}

private:
    typedef bool (*FilterOut)(TradeRecord*);
    boost::property_tree::ptree Ping();
    boost::property_tree::ptree OpenOrder(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree AddOrder(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree UpdateOrder(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree CloseOrder(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree Deposit(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree GetUserRecord(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree UpdateUserRecord(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree ChangePassword(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree CheckPassword(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree GetMargin(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree GetMarginInfo(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree GetOrder(const boost::property_tree::ptree& pt);
    boost::property_tree::ptree AddUser(const boost::property_tree::ptree& pt);
    inline void GetOpenOrders(boost::property_tree::ptree pt, std::string& response);
    inline void GetPendingOrders(boost::property_tree::ptree pt, std::string& response);
    inline void GetClosedOrders(boost::property_tree::ptree pt, std::string& response);
    inline boost::property_tree::ptree IsOpening(const boost::property_tree::ptree& pt);
    inline boost::property_tree::ptree TradeTime(const boost::property_tree::ptree& pt);
    inline boost::property_tree::ptree GetSymbolList(const boost::property_tree::ptree& pt);

private:
    void _GetOpenOrders(const boost::property_tree::ptree& pt, FilterOut filter, std::string& response);
    void _GetClosedOrders(const boost::property_tree::ptree& pt, FilterOut filter, std::string& response);
    void AppendTradeRecordJsonStr(TradeRecord* trade_record, std::string& response);
    void SetResponseJson(boost::property_tree::ptree& response, const std::string& request, bool result,
                         const ErrorCode* error_code);
};

#endif  // !_JSON_HANDLER_H_