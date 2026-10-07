#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <map>
#include <string>

class HttpResponse {
public:
    int statusCode = 200;
    std::string contentType = "text/html; charset=utf-8";
    std::string body;
    std::map<std::string, std::string> headers;  // extra headers (e.g. Allow)

    static HttpResponse make(int code, std::string body, std::string contentType);
    static HttpResponse error(int code);  // small HTML error page
    static const char* reasonPhrase(int code);

    // Builds the bytes to send. For HEAD requests pass includeBody = false:
    // Content-Length still reports the real size, but no body follows.
    std::string serialize(bool includeBody = true) const;
};

#endif
