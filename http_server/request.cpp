#include "request.h"

void http::server::request::reset() {
    method.clear();
    uri.clear();
    headers.clear();
    body.clear();
}
