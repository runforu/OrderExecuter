#ifndef _HTTP_SERVER_REQUEST_INTERCEPTION_H_
#define _HTTP_SERVER_REQUEST_INTERCEPTION_H_

#include "reply.h"
#include "request.h"

namespace http {
namespace server {

class request_interception {
public:
    virtual bool handle(const request& req) = 0;
    virtual ~request_interception(){};
};

}  // namespace server
}  // namespace http
#endif  // !_HTTP_SERVER_REQUEST_INTERCEPTION_H_
