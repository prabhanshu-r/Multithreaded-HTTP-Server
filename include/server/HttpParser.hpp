#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include <string>

#include "server/HttpRequest.hpp"

class HttpParser {
public:
    HttpRequest parse(const std:: string& request);
};

#endif