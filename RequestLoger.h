#ifndef _REQUEST_LOGER_H_
#define _REQUEST_LOGER_H_

#include "http_server/request_interception.h"

struct http::server::request;

class RequestLoger : public http::server::request_interception {
public:
    bool handle(const http::server::request& req);
    ~RequestLoger() {}

};

#endif  // !_REQUEST_LOGER_H_