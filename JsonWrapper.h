#ifndef _HTTP_SERVER_JSON_HELPER_H_
#define _HTTP_SERVER_JSON_HELPER_H_

// ptree depends on boost::spirit, boost::spirit is not thread safe
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class JsonWrapper {
public:
    static boost::property_tree::ptree parse_json(std::string json_str);

    static std::string to_json(boost::property_tree::ptree& pt);
};

#endif  // !_HTTP_SERVER_JSON_HELPER_H_
