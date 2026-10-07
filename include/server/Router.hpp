#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"
#include "server/StaticFileHandler.hpp"

// Decides what to do with a request: a built-in route or a static file.
class Router {
public:
    explicit Router(const StaticFileHandler& files) : files_(files) {}

    HttpResponse route(const HttpRequest& request) const;

private:
    const StaticFileHandler& files_;
};

#endif
