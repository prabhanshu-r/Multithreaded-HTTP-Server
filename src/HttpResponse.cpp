#include "server/HttpResponse.hpp"

std::string HttpResponse::toString() const {

    return 
    "HTTP/1.1" +
    std::to_string(statusCode) +
    " " +
    statusMessge +
    "\r\n" + "Content-Type : text/html\r\n"
    + "Content-Length: "+
    std::to_string(body.size()) +
    "\r\n\r\n" +
    body;
}