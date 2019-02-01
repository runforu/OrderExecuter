#include "DefaultHandler.h"
#include "JsonHandler.h"
#include "MiscJsonHandler.h"
#include "RequestHandlerProviderImp.h"
#include "http_server/request_handler.h"

RequestHandlerProviderImp::RequestHandlerProviderImp() {
    m_request_handlers.push_back(new JsonHandler());
    m_request_handlers.push_back(new MiscJsonHandler());
    m_default_handler = new DefaultHandler();
}

RequestHandlerProviderImp::~RequestHandlerProviderImp() {
    for (std::vector<http::server::request_handler*>::iterator it = m_request_handlers.begin(); it != m_request_handlers.end();
         it++) {
        if (*it != NULL) {
            delete *it;
            *it = NULL;
        }
    }
    delete m_default_handler;
}

std::vector<http::server::request_handler*> RequestHandlerProviderImp::get_handlers() {
    return m_request_handlers;
}

http::server::request_handler* RequestHandlerProviderImp::get_default_handlers() {
    return m_default_handler;
}
