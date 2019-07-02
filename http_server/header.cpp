#include "header.h"
namespace http {
namespace server {

const header header::json_content_type("content-type", "application/json");
const std::string header::content_length("content-length");

}  // namespace server
}  // namespace http
