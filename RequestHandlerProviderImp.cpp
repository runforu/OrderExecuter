#include "RequestHandlerProviderImp.h"
#include "file_handler.h"
#include "http_server/request_handler.h"
#include "json_handler.h"

RequestHandlerProviderImp::RequestHandlerProviderImp() {
    m_request_handlers.push_back(new http::server::json_handler());
    m_request_handlers.push_back(new http::server::file_handler());
}

RequestHandlerProviderImp::~RequestHandlerProviderImp() {
    for (std::vector<http::server::request_handler*>::iterator it = m_request_handlers.begin(); it != m_request_handlers.end();
         it++) {
        if (*it != NULL) {
            delete *it;
            *it = NULL;
        }
    }
}

std::vector<http::server::request_handler*> RequestHandlerProviderImp::get_handlers() {
    return m_request_handlers;
}
