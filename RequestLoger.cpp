
#include "Loger.h"
#include "RequestLoger.h"
#include "http_server/request.h"

bool RequestLoger::handle(const http::server::request& req) {
    LOG("Request: %s", req.body.c_str());
    return false;
}
