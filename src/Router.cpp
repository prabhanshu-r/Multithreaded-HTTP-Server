#include "server/Router.hpp"

HttpResponse Router::route(const HttpRequest& request) const {
    // This server only reads: GET fetches a page, HEAD fetches just the headers.
    if (request.method != "GET" && request.method != "HEAD") {
        HttpResponse response = HttpResponse::error(405);
        response.headers["Allow"] = "GET, HEAD";
        return response;
    }

    // Built-in route: handy for load balancers and uptime monitors.
    if (request.path == "/health") {
        return HttpResponse::make(200, "ok\n", "text/plain; charset=utf-8");
    }

    return files_.serve(request.path);
}
