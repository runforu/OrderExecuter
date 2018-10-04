#ifndef _HTTP_SERVER_JSON_HELPER_H_
#define _HTTP_SERVER_JSON_HELPER_H_

// ptree depends on boost::spirit, boost::spirit is not thread safe
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace boost::property_tree;

namespace http {
namespace server {

class json_wrapper {
public:
    static ptree parse_json(std::string json_str);

    static std::string to_json(ptree& pt);
};

}  // namespace server
}  // namespace http

#endif  // !_HTTP_SERVER_JSON_HELPER_H_
