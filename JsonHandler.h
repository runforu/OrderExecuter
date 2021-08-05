#ifndef _JSON_HANDLER_H_
#define _JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "http_server/request_handler.h"

struct TradeRecord;
struct ErrorCode;

class JsonHandler : public http::server::request_handler {
public:
    virtual int get_priority() const override;

    virtual bool can_handle(const http::server::request& req) const override;

    virtual bool handle(const http::server::request& req, http::server::reply& rep) const override;

    virtual ~JsonHandler() {}

private:
    typedef bool (*FilterOut)(TradeRecord*);

    void Ping(boost::property_tree::ptree& pt) const;

    void BinaryOption(boost::property_tree::ptree& pt) const;

    void OpenOrder(boost::property_tree::ptree& pt) const;

    void AddOrder(boost::property_tree::ptree& pt) const;

    void UpdateOrder(boost::property_tree::ptree& pt) const;

    void CloseOrder(boost::property_tree::ptree& pt) const;

    void Deposit(boost::property_tree::ptree& pt) const;

    void GetUserRecord(boost::property_tree::ptree& pt) const;

    void UpdateUserRecord(boost::property_tree::ptree& pt) const;

    void ChangePassword(boost::property_tree::ptree& pt) const;

    void CheckPassword(boost::property_tree::ptree& pt) const;

    void GetMargin(boost::property_tree::ptree& pt) const;

    void GetMarginInfo(boost::property_tree::ptree& pt) const;

    void GetOrder(boost::property_tree::ptree& pt) const;

    void AddUser(boost::property_tree::ptree& pt) const;

    void GetOpenOrders(const boost::property_tree::ptree& pt, std::string& response) const;

    void GetPendingOrders(const boost::property_tree::ptree& pt, std::string& response) const;

    void GetClosedOrders(const boost::property_tree::ptree& pt, std::string& response) const;

    void IsOpening(boost::property_tree::ptree& pt) const;

    void TradeTime(boost::property_tree::ptree& pt) const;

    void GetSymbolList(boost::property_tree::ptree& pt) const;

private:
    void _GetOpenOrders(const boost::property_tree::ptree& pt, FilterOut filter, std::string& response) const;

    void _GetClosedOrders(const boost::property_tree::ptree& pt, FilterOut filter, std::string& response) const;

    void AppendTradeRecordJsonStr(TradeRecord* trade_record, std::string& response) const;

    void SetResponseJson(boost::property_tree::ptree& response, const std::string& request, bool result,
                         const ErrorCode* error_code) const;
};

#endif  // !_JSON_HANDLER_H_