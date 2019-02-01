#ifndef _MISC_JSON_HANDLER_H_
#define _MISC_JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "http_server/request_handler.h"

struct TradeRecord;

class MiscJsonHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;
    virtual bool can_handle(const http::server::request& req);
    virtual bool handle(const http::server::request& req, http::server::reply& rep);
    virtual ~MiscJsonHandler() {}

private:
    boost::property_tree::ptree RequestChart(boost::property_tree::ptree pt);

private:
};

#endif  // !_MISC_JSON_HANDLER_H_