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
        socket.close();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}

static std::string content(
    "{ \"request\": \"RequestChart\", \"symbol\": \"USDJPY\", \"period\": 1, \"mode\": \"CHART_RANGE_IN\", \"start\": 0, "
    "\"end\": 1548992369, \"timestamp\": 1 }");

static std::string content1(
    "{ \"request\": \"OpenOrder\", \"login\": \"6\", \"ip\": \"0.0.0.0\", \"symbol\": \"USDJPY\", \"cmd\": \"OP_BUY\", "
    "\"volume\": 1, \"open_price\": 110.0, \"sl\": 0.0, \"tp\": 0.0, \"comment\": \"test OpenOrder\" }");

static std::string content2("{ \"request\": \"GetOpenOrders\", \"login\": \"5\"}");

static std::string content3("{\"request\": \"IsOpening\", \"symbol\" : \"USDJPY\",\"time\" : 0}");

static std::string url("http://120.79.58.246:8080");

void RequestChart() {
    sync_client(url, content);
}

void OpenOrder() {
    sync_client(url, content1);
}

void GetOpenOrders() {
    sync_client(url, content2);
}

void IsOpening() {
    sync_client(url, content3);
}

int main() {
    std::string s;
    s.reserve(46137444);
    std::cout << s.capacity() << std::endl;
    // return 0;

    std::vector<boost::thread> vt;

    for (int j = 0; j < 1; j++) {
        for (int i = 0; i < 1000; i++) {
            try {
                // vt.push_back(boost::thread(RequestChart));
                // vt.push_back(boost::thread(OpenOrder));
                vt.push_back(boost::thread(GetOpenOrders));
                // vt.push_back(boost::thread(IsOpening));
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

        vt.clear();
    }

    std::cout << "test complete.\n";
    getchar();
}
