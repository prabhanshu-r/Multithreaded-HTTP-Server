#include "server/HttpResponse.hpp"

#include <ctime>

namespace {

std::string httpDate() {
    std::time_t now = std::time(nullptr);
    std::tm utc{};
    gmtime_r(&now, &utc);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &utc);
    return buffer;
}

}  // namespace

const char* HttpResponse::reasonPhrase(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 414: return "URI Too Long";
        case 431: return "Request Header Fields Too Large";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        case 505: return "HTTP Version Not Supported";
        default: return "Unknown";
    }
}

HttpResponse HttpResponse::make(int code, std::string body, std::string contentType) {
    HttpResponse response;
    response.statusCode = code;
    response.body = std::move(body);
    response.contentType = std::move(contentType);
    return response;
}

HttpResponse HttpResponse::error(int code) {
    std::string title = std::to_string(code) + " " + reasonPhrase(code);
    std::string page = "<!DOCTYPE html><html><head><title>" + title +
                       "</title></head><body><h1>" + title + "</h1></body></html>\n";
    return make(code, page, "text/html; charset=utf-8");
}

std::string HttpResponse::serialize(bool includeBody) const {
    std::string out;
    out += "HTTP/1.1 " + std::to_string(statusCode) + " " + reasonPhrase(statusCode) + "\r\n";
    out += "Date: " + httpDate() + "\r\n";
    out += "Server: MultithreadedHTTPServer\r\n";
    out += "Content-Type: " + contentType + "\r\n";
    out += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    out += "Connection: close\r\n";
    out += "X-Content-Type-Options: nosniff\r\n";
    for (const auto& header : headers) {
        out += header.first + ": " + header.second + "\r\n";
    }
    out += "\r\n";
    if (includeBody) out += body;
    return out;
}
