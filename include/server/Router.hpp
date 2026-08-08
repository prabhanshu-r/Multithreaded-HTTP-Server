#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

class Router {
public:
    HttpResponse route(const HttpRequest& request);
};

#endif