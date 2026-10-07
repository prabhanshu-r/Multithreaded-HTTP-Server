#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

// Thread-safe logger: one line per message, with a UTC timestamp.
class Logger {
public:
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static void write(const char* level, const std::string& message, bool toStderr);
};

#endif
