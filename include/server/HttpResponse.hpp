#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>

class HttpResponse {
public:
    int statusCode{200};
    std::string statusMessge{"OK"};
    std::string body;

    std::string toString() const;
};

#endif