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
        // std::cout << "Request complete. \n";
        socket.close();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}

int DiffTime(SYSTEMTIME time1, SYSTEMTIME time2) {
    return (time1.wSecond - time2.wSecond) * 1000 + time1.wMilliseconds - time2.wMilliseconds +
           ((time1.wMinute != time2.wMinute) ? 60 * 1000 : 0);
}

int sync_client1(const std::string& url, const std::string& m_content) {
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

        SYSTEMTIME time0;
        GetLocalTime(&time0);

        boost::asio::connect(socket, endpoints);

        SYSTEMTIME time1;
        GetLocalTime(&time1);

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

        SYSTEMTIME time2;
        GetLocalTime(&time2);

        boost::asio::streambuf response;
        boost::system::error_code error;
        boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error);

        boost::asio::streambuf::const_buffers_type cbt = response.data();
        std::string data(boost::asio::buffers_begin(cbt), boost::asio::buffers_end(cbt));
        if (data.find("Unknown error") != std::string::npos) {
            std::cout << "Unknown error. \n";
        }

        SYSTEMTIME time3;
        GetLocalTime(&time3);

        std::cout << "GetMargin takes " << DiffTime(time3, time0) << " connect: " << DiffTime(time1, time0)
                  << " write: " << DiffTime(time2, time1) << " read: " << DiffTime(time3, time2) << std::endl;
        // std::cout << "Request complete. \n";
        socket.close();
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}

static std::string url("http://47.52.32.43:8999");
//static std::string url("http://120.79.58.246:8080");

void RequestChart() {
    static std::string content(
        "{ \"request\": \"RequestChart\", \"symbol\": \"USDJPY\", \"period\": 5, \"mode\": \"CHART_RANGE_IN\", \"start\": 0, "
        "\"end\": 1558992369, \"timestamp\": 1 }");
    sync_client(url, content);
}

void OpenOrder() {
    static std::string content(
        "{ \"request\": \"OpenOrder\", \"login\": \"6\", \"ip\": \"0.0.0.0\", \"symbol\": \"USDJPY\", \"cmd\": \"OP_BUY\", "
        "\"volume\": 1, \"open_price\": 110.0, \"sl\": 0.0, \"tp\": 0.0, \"comment\": \"test OpenOrder\" }");
    sync_client(url, content);
}

void GetOpenOrders() {
    static std::string content("{ \"request\": \"GetOpenOrders\", \"login\": 4100286}");
    sync_client(url, content);
}

void GetClosedOrders() {
    static std::string content("{ \"request\": \"GetClosedOrders\", \"login\": 4100286}");
    sync_client(url, content);
}

void IsOpening() {
    static std::string content("{\"request\": \"IsOpening\", \"symbol\" : \"USDJPY\",\"time\" : 0}");
    sync_client(url, content);
}

void GetMargin1() {
    static std::string content("{ \"request\": \"GetMargin\", \"login\": 4100286}");
    sync_client1(url, content);
}

void GetMargin() {
    static std::string content("{ \"request\": \"GetMargin\", \"login\": 4100286}");
    sync_client(url, content);
}

int main() {
    std::vector<boost::thread> vt;
    for (int i = 0; i < 200; i++) {
        for (int i = 0; i < 2000; i++) {
            try {
                //vt.push_back(boost::thread(RequestChart));
                // vt.push_back(boost::thread(OpenOrder));
                //vt.push_back(boost::thread(GetOpenOrders));
                //vt.push_back(boost::thread(GetClosedOrders));
                vt.push_back(boost::thread(GetMargin1));
            } catch (boost::exception& e) {
                std::cerr << "create thread error: " << boost::current_exception_diagnostic_information() << std::endl;
            }
        }
        //GetMargin1();
        for (std::vector<boost::thread>::iterator it = vt.begin(); it != vt.end(); it++) {
            try {
                (*it).join();
            }
            catch (boost::exception& e) {
                std::cerr << "join error: " << boost::current_exception_diagnostic_information() << std::endl;
            }
        }
        vt.clear();
        std::cerr << "--------------------------------------------\n";
    }

    for (std::vector<boost::thread>::iterator it = vt.begin(); it != vt.end(); it++) {
        try {
            (*it).join();
        } catch (boost::exception& e) {
            std::cerr << "join error: " << boost::current_exception_diagnostic_information() << std::endl;
        }
    }
    std::cout << "test complete.\n";
}
