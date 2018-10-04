#include "json_wrapper.h"
namespace http {
namespace server {

ptree json_wrapper::parse_json(std::string json_str) {
    ptree tree;
    std::stringstream ss(json_str);
    try {
        read_json(ss, tree);
    } catch (ptree_error& e) {
    }
    return tree;
}

std::string json_wrapper::to_json(ptree& pt) {
    std::stringstream ss;
    write_json(ss, pt);
    return ss.str();
}

}  // namespace server
}  // namespace http
