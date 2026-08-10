#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <string>

class Router {
public:
    HttpResponse route(const HttpRequest& request);
    std::string getFilePath(const HttpRequest& request);
};

#endif