#ifndef _HTTP_SERVER_REQUEST_HANDLER_H_
#define _HTTP_SERVER_REQUEST_HANDLER_H_

#include "reply.h"
#include "request.h"

namespace http {
namespace server {

class request_handler {
public:
    virtual int get_priority() const = 0;
    virtual bool can_handle(const request& req) = 0;
    virtual bool handle(const request& req, reply& rep) = 0;
    virtual ~request_handler(){};
};

}  // namespace server
}  // namespace http
#endif  // !_HTTP_SERVER_REQUEST_HANDLER_H_
