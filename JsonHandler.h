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

    void Ping(boost::property_tree::ptree& pt);

    void OpenOrder(boost::property_tree::ptree& pt);

    void AddOrder(boost::property_tree::ptree& pt);

    void UpdateOrder(boost::property_tree::ptree& pt);

    void CloseOrder(boost::property_tree::ptree& pt);

    void Deposit(boost::property_tree::ptree& pt);

    void GetUserRecord(boost::property_tree::ptree& pt);

    void UpdateUserRecord(boost::property_tree::ptree& pt);

    void ChangePassword(boost::property_tree::ptree& pt);

    void CheckPassword(boost::property_tree::ptree& pt);

    void GetMargin(boost::property_tree::ptree& pt);

    void GetMarginInfo(boost::property_tree::ptree& pt);

    void GetOrder(boost::property_tree::ptree& pt);

    void AddUser(boost::property_tree::ptree& pt);

    void GetOpenOrders(const boost::property_tree::ptree& pt, std::string& response);

    void GetPendingOrders(const boost::property_tree::ptree& pt, std::string& response);

    void GetClosedOrders(const boost::property_tree::ptree& pt, std::string& response);

    void IsOpening(boost::property_tree::ptree& pt);

    void TradeTime(boost::property_tree::ptree& pt);

    void GetSymbolList(boost::property_tree::ptree& pt);

private:
    void _GetOpenOrders(const boost::property_tree::ptree& pt, FilterOut filter, std::string& response);

    void _GetClosedOrders(const boost::property_tree::ptree& pt, FilterOut filter, std::string& response);

    void AppendTradeRecordJsonStr(TradeRecord* trade_record, std::string& response);

    void SetResponseJson(boost::property_tree::ptree& response, const std::string& request, bool result,
                         const ErrorCode* error_code);
};

#endif  // !_JSON_HANDLER_H_