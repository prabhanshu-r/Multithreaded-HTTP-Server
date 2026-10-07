#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <map>
#include <string>

class HttpRequest {
public:
    std::string method;   // "GET"
    std::string target;   // raw target as sent: "/a%20b?x=1"
    std::string path;     // decoded path without query: "/a b"
    std::string query;    // "x=1" (without the '?')
    std::string version;  // "HTTP/1.1"
    std::map<std::string, std::string> headers;  // names are lower-case
};

#endif
