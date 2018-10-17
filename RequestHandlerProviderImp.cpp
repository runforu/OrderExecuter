#include "RequestHandlerProviderImp.h"
#include "FileHandler.h"
#include "http_server/request_handler.h"
#include "JsonHandler.h"

RequestHandlerProviderImp::RequestHandlerProviderImp() {
    m_request_handlers.push_back(new JsonHandler());
    m_request_handlers.push_back(new FileHandler());
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
