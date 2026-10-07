#include "server/Logger.hpp"

#include <ctime>
#include <iostream>
#include <mutex>

namespace {

std::mutex logMutex;  // keeps lines from different threads from interleaving

std::string timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm utc{};
    gmtime_r(&now, &utc);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buffer;
}

}  // namespace

void Logger::write(const char* level, const std::string& message, bool toStderr) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ostream& out = toStderr ? std::cerr : std::cout;
    out << timestamp() << " [" << level << "] " << message << std::endl;
}

void Logger::info(const std::string& message) { write("INFO", message, false); }
void Logger::warn(const std::string& message) { write("WARN", message, true); }
void Logger::error(const std::string& message) { write("ERROR", message, true); }
