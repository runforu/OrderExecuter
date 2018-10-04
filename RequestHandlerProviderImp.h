#ifndef _REQUESTHANDLERPROVIDERIMP_H_
#define _REQUESTHANDLERPROVIDERIMP_H_

#include <vector>
#include "http_server/request_handler_provider.h"

class RequestHandlerProviderImp: public http::server::request_handler_provider
{
public:
    RequestHandlerProviderImp();
    ~RequestHandlerProviderImp();
    std::vector<http::server::request_handler*> get_handlers();

private:
    std::vector<http::server::request_handler*> m_request_handlers;
};

#endif // !_REQUESTHANDLERPROVIDERIMP_H_
