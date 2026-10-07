#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include <string>

#include "server/HttpRequest.hpp"

class HttpParser {
public:
    // Parses the request line and headers from `raw`.
    // On failure returns false and sets `errorStatus` (400, 414, 431, 505...).
    bool parse(const std::string& raw, HttpRequest& request, int& errorStatus) const;
};

#endif
