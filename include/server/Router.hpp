#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "server/HttpRequest.hpp"

#include <string>

class Router {
public:
    std::string getFilePath(const HttpRequest& request);
};

#endif