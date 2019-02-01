#ifndef _DEFAULTHANDLER_H_

#define _DEFAULTHANDLER_H_

#include "http_server/reply.h"
#include "http_server/request.h"
#include "http_server/request_handler.h"

class DefaultHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;
    virtual bool handle(const http::server::request& req, http::server::reply& rep);
    virtual ~DefaultHandler() {}

private:
    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string& in, std::string& out);

    // Inherited via request_handler
    virtual bool can_handle(const http::server::request & req) override;
};

#endif  // !_DEFAULTHANDLER_H_
