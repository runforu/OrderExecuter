#include "header.h"
namespace http {
namespace server {

const header header::response_json_content_type("Content-Type", "application/json");
const header header::json_content_type("content-type", "application/json");
const header header::header_connection("Connection", "keep-alive");
const header header::keep_alive("Keep-Alive", "timeout=180");
const std::string header::content_length("content-length");
const std::string header::response_content_length("Content-Length");

}  // namespace server
}  // namespace http
