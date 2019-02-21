#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/thread.hpp>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <regex>
#include <string>

int sync_client(const std::string& url, const std::string& m_content) {
    std::regex ex("(http)://([^/ :]+):?([^/ ]*)(/?.*)");
    std::smatch what;
    if (!std::regex_match(url, what, ex)) {
        return -1;
    }

    std::string scheme = what[1];
    std::string host = what[2];
    std::string port = what[3];
    std::string path = what[4];

    if (port.empty()) {
        port = "80";
    }
    if (path.empty()) {
        path = "/";
    }

    try {
        boost::asio::io_context io_context;

        // Get a list of endpoints corresponding to the server name.
        boost::asio::ip::tcp::resolver resolver(io_context);

        boost::asio::ip::tcp::resolver::query query(host, port);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(query);

        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n";
        request_stream << "Content-Length: " << m_content.length() << "\r\n";
        request_stream << "Content-Type: application/json\r\n\r\n";
        request_stream << m_content;

        boost::asio::write(socket, request);

        boost::asio::streambuf response;
        boost::system::error_code error;
        boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error);
        std::cout << "Request complete. \n";
        // socket.close();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}

static std::string content(
    "{ \"request\": \"RequestChart\", \"symbol\": \"USDJPY\", \"period\": 1, \"mode\": \"CHART_RANGE_IN\", \"start\": 0, "
    "\"end\": 1548992369, \"timestamp\": 1 }");

static std::string content1(
    "{ \"request\": \"OpenOrder\", \"login\": \"5\", \"ip\": \"0.0.0.0\", \"symbol\": \"USDJPY\", \"cmd\": \"OP_BUY\", "
    "\"volume\": 1, \"open_price\": 110.0, \"sl\": 0.0, \"tp\": 0.0, \"comment\": \"test OpenOrder\" }");

static std::string url("http://120.79.58.246:8080");

void request() {
    sync_client(url, content);
}

void request1() {
    sync_client(url, content1);
}

void emulate(boost::property_tree::ptree& tree) {
    int total = 10000;
    tree.put("count", total);
    tree.put("timesign", 1);
    boost::property_tree::ptree rate_infos;
    for (int i = 0; i < total; i++) {
        boost::property_tree::ptree rate_info;
        rate_info.put("open_price", 1.2);
        rate_info.put("high", 1);
        rate_info.put("low", 1);
        rate_info.put("close", 1);
        rate_info.put("time", 1);
        rate_info.put("volume", 111);
        rate_infos.push_back(std::make_pair("", rate_info));
    }
    tree.add_child("rate_infos", rate_infos);
}

int main() {
    /*
    int chunk = 1024 * 1024;
    boost::property_tree::ptree tree;
    for (int i = 0; i < 3000; i++) {
        int* p = (int*)malloc(chunk);
        if (p == NULL) {
            chunk /= 2;
        }
    }
    try {
        tree.put("aaaaaaaaa", "bbbbbbbbbbbbbb");
        tree.put("aaaasaaaaa", "bbbbbbbbbbbbbb");
    } catch (...) {
        std::cerr << "create thread error: "  << std::endl;
    }

    getchar();
    return 0;
    */
    boost::property_tree::ptree tree[256];
    for (int i = 0; i < 256; i++) {
        emulate(tree[i]);
    }
    getchar();
    return 0;

    std::vector<boost::thread> vt;

    for (int i = 0; i < 10000; i++) {
        try {
            vt.push_back(boost::thread(request));
            vt.push_back(boost::thread(request1));
        } catch (boost::exception& e) {
            std::cerr << "create thread error: " << boost::current_exception_diagnostic_information() << std::endl;
        }
    }

    for (std::vector<boost::thread>::iterator it = vt.begin(); it != vt.end(); it++) {
        try {
            (*it).join();
        } catch (boost::exception& e) {
            std::cerr << "join error: " << boost::current_exception_diagnostic_information() << std::endl;
        }
    }

    std::cout << "test complete.\n";
    getchar();
}
