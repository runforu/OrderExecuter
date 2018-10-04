#ifndef _REQUEST_HANDLER_PROVIDER_H_
#define _REQUEST_HANDLER_PROVIDER_H_

#include <string>
#include <vector>
#include "request_handler.h"

namespace http {
namespace server {

class request_handler_provider {
public:
    virtual ~request_handler_provider(){};
    virtual std::vector<request_handler*> get_handlers() = 0;
};

}  // namespace server
}  // namespace http

#endif  // _REQUEST_HANDLER_PROVIDER_H_