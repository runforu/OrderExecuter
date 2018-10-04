#ifndef _HTTP_SERVER_JSON_HANDLER_H_
#define _HTTP_SERVER_JSON_HANDLER_H_

#include <boost/property_tree/ptree.hpp>
#include "http_server/reply.h"
#include "http_server/request.h"
#include "http_server/request_handler.h"

namespace http {
namespace server {

class json_handler : public request_handler {
public:
    virtual int get_priority() const;
    virtual bool handle(const request& req, reply& rep);
    virtual ~json_handler() {}
    json_handler() {}

private:
    boost::property_tree::ptree OpenOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree AddOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree UpdateOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree CloseOrder(boost::property_tree::ptree pt);
    boost::property_tree::ptree Deposit(boost::property_tree::ptree pt);
    boost::property_tree::ptree GetUserRecord(boost::property_tree::ptree pt);
    boost::property_tree::ptree UpdateUserRecord(boost::property_tree::ptree pt);
};

}  // namespace server
}  // namespace http
#endif  // !_HTTP_SERVER_JSON_HANDLER_H_