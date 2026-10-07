#include "server/HttpParser.hpp"

#include <cctype>
#include <sstream>

namespace {

const std::size_t kMaxTargetLength = 2048;
const std::size_t kMaxHeaderCount = 100;

std::string trim(const std::string& text) {
    const char* whitespace = " \t";
    std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Turns "%20" into ' ' etc. Rejects bad escapes and control characters (NUL, \n...).
bool percentDecode(const std::string& in, std::string& out) {
    out.clear();
    for (std::size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '%') {
            if (i + 2 >= in.size()) return false;
            int high = hexValue(in[i + 1]);
            int low = hexValue(in[i + 2]);
            if (high < 0 || low < 0) return false;
            c = static_cast<char>(high * 16 + low);
            i += 2;
        }
        unsigned char u = static_cast<unsigned char>(c);
        if (u < 0x20 || u == 0x7f) return false;
        out.push_back(c);
    }
    return true;
}

bool isMethodToken(const std::string& method) {
    if (method.empty() || method.size() > 16) return false;
    for (char c : method) {
        if (c < 'A' || c > 'Z') return false;
    }
    return true;
}

}  // namespace

bool HttpParser::parse(const std::string& raw, HttpRequest& request, int& errorStatus) const {
    errorStatus = 400;

    std::size_t headEnd = raw.find("\r\n\r\n");
    if (headEnd == std::string::npos) return false;
    const std::string head = raw.substr(0, headEnd);

    // ---- request line: METHOD SP TARGET SP VERSION ----
    std::size_t lineEnd = head.find("\r\n");
    const std::string requestLine = head.substr(0, lineEnd);

    std::istringstream line(requestLine);
    std::string extra;
    if (!(line >> request.method >> request.target >> request.version)) return false;
    if (line >> extra) return false;  // too many parts

    if (!isMethodToken(request.method)) return false;

    if (request.target.size() > kMaxTargetLength) {
        errorStatus = 414;
        return false;
    }
    if (request.target.empty() || request.target[0] != '/') return false;

    if (request.version != "HTTP/1.1" && request.version != "HTTP/1.0") {
        errorStatus = (request.version.rfind("HTTP/", 0) == 0) ? 505 : 400;
        return false;
    }

    std::string rawPath = request.target;
    std::size_t question = rawPath.find('?');
    if (question != std::string::npos) {
        request.query = rawPath.substr(question + 1);
        rawPath.erase(question);
    }
    if (!percentDecode(rawPath, request.path)) return false;

    // ---- headers: "Name: value" ----
    request.headers.clear();
    std::size_t pos = (lineEnd == std::string::npos) ? head.size() : lineEnd + 2;
    std::size_t count = 0;
    while (pos < head.size()) {
        std::size_t next = head.find("\r\n", pos);
        std::string headerLine = head.substr(pos, next == std::string::npos ? std::string::npos : next - pos);
        pos = (next == std::string::npos) ? head.size() : next + 2;

        if (++count > kMaxHeaderCount) {
            errorStatus = 431;
            return false;
        }
        std::size_t colon = headerLine.find(':');
        if (colon == std::string::npos || colon == 0) return false;

        std::string name = headerLine.substr(0, colon);
        if (name.find_first_of(" \t") != std::string::npos) return false;  // no space before ':'
        for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        request.headers[name] = trim(headerLine.substr(colon + 1));
    }

    // HTTP/1.1 requires a Host header.
    if (request.version == "HTTP/1.1" && request.headers.find("host") == request.headers.end()) {
        return false;
    }

    errorStatus = 0;
    return true;
}
