#include "server/Config.hpp"

#include <cstdlib>
#include <fstream>

#include "server/Logger.hpp"

namespace {

std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

// Parses a whole-string integer within [min, max].
bool parseNumber(const std::string& text, long long min, long long max, long long& out) {
    try {
        std::size_t used = 0;
        long long value = std::stoll(text, &used);
        if (used != text.size() || value < min || value > max) return false;
        out = value;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void applySetting(Config& config, const std::string& key, const std::string& value, int lineNo) {
    long long number = 0;
    const std::string where = "config line " + std::to_string(lineNo) + ": ";

    if (key == "host") {
        config.host = value;
    } else if (key == "document_root") {
        config.documentRoot = value;
    } else if (key == "port") {
        if (parseNumber(value, 1, 65535, number)) config.port = static_cast<int>(number);
        else Logger::warn(where + "port must be 1-65535, keeping " + std::to_string(config.port));
    } else if (key == "workers") {
        if (parseNumber(value, 1, 256, number)) config.workers = static_cast<std::size_t>(number);
        else Logger::warn(where + "workers must be 1-256, keeping " + std::to_string(config.workers));
    } else if (key == "queue_limit") {
        if (parseNumber(value, 1, 100000, number)) config.queueLimit = static_cast<std::size_t>(number);
        else Logger::warn(where + "queue_limit must be 1-100000");
    } else if (key == "max_request_size") {
        if (parseNumber(value, 512, 1048576, number)) config.maxRequestSize = static_cast<std::size_t>(number);
        else Logger::warn(where + "max_request_size must be 512-1048576");
    } else if (key == "read_timeout") {
        if (parseNumber(value, 1, 300, number)) config.readTimeoutSeconds = static_cast<int>(number);
        else Logger::warn(where + "read_timeout must be 1-300");
    } else {
        Logger::warn(where + "unknown setting '" + key + "' ignored");
    }
}

}  // namespace

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            Logger::warn("config line " + std::to_string(lineNo) + ": expected key = value");
            continue;
        }
        applySetting(*this, trim(line.substr(0, equals)), trim(line.substr(equals + 1)), lineNo);
    }
    return true;
}

void Config::applyEnvironment() {
    const char* portText = std::getenv("PORT");
    if (!portText) return;

    long long number = 0;
    if (parseNumber(portText, 1, 65535, number)) {
        port = static_cast<int>(number);
    } else {
        Logger::warn(std::string("ignoring invalid PORT environment variable: ") + portText);
    }
}
