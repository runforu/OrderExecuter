#ifndef _MISC_JSON_HANDLER_H_
#define _MISC_JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "ManagerApi.h"
#include "http_server/request_handler.h"

struct TradeRecord;

class MiscJsonHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;

    virtual bool can_handle(const http::server::request& req) const override;

    virtual bool handle(const http::server::request& req, http::server::reply& rep) const override;

    virtual ~MiscJsonHandler() {}

private:
    // free the memory; the chart data may be huge, so avoid using ptree.
    void RequestChart(boost::property_tree::ptree pt, std::string& content) const;
};

#endif  // !_MISC_JSON_HANDLER_H_