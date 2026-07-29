#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>

class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
};

#endif