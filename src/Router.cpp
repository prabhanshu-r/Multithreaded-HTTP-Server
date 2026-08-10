#include "server/Router.hpp"
#include "server/StaticFileHandler.hpp"

HttpResponse Router::route(const HttpRequest& request) {
    // HttpResponse response;
    StaticFileHandler fileHandler;


    // if(request.path == "/") {
    //     response.body = "<html> <h1> Home Page </h1> </html>";
    // } else if(request.path == "/about") {
    //     response.body = "<html> <h1> About Page </h1> </html>";
    // } else if(request.path == "/contact") {
    //     response.body = "<html> <h1> Contact Page </h1> </html>";
    // }else {
    //     response.statusCode = 404;
    //     response.statusMessge = "Not Found";

    //     response.body = "<html> <h1> 404 Not Found </h1> </html>";
    // }

    if (request.path == "/") return "public/index.html";
    if (request.path == "/about") return "public/about.html";
    if (request.path == "/contact") return "public/contact.html";

    return "";
}