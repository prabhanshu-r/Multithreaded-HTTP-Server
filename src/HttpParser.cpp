#include "server/HttpParser.hpp"

#include <sstream>

HttpRequest HttpParser::parse(const std::string& request) {
    HttpRequest httpRequest;

    std::stringstream stream(request);

    std::string requestLine;

    std::getline(stream, requestLine);

    std::stringstream lineStream(requestLine);

    lineStream >> httpRequest.method;
    lineStream >> httpRequest.path;
    lineStream >> httpRequest.version;

    return httpRequest;
}

// curl http://127.0.0.1:8080/about