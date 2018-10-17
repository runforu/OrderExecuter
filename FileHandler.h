#ifndef _HTTP_SERVER_FILE_HANDLER_H_
#define _HTTP_SERVER_FILE_HANDLER_H_

#include "http_server/reply.h"
#include "http_server/request.h"
#include "http_server/request_handler.h"

class FileHandler : public http::server::request_handler {
public:
    virtual int get_priority() const;
    virtual bool handle(const http::server::request& req, http::server::reply& rep);
    virtual ~FileHandler() {}
    FileHandler() {}

private:
    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string& in, std::string& out);
};

#endif  // !_HTTP_SERVER_FILE_HANDLER_H_
