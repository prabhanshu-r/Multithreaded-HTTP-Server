#include "server/Router.hpp"

HttpResponse Router::route(const HttpRequest& request) {
    HttpResponse response;

    if(request.path == "/") {
        response.body = "<html> <h1> Home Page </h1> </html>";
    } else if(request.path == "/about") {
        response.body = "<html> <h1> About Page </h1> </html>";
    } else if(request.path == "/contact") {
        response.body = "<html> <h1> Contact Page </h1> </html>";
    }else {
        response.statusCode = 404;
        response.statusMessge = "Not Found";

        response.body = "<html> <h1> 404 Not Found </h1> </html>";
    }

    return response;
}