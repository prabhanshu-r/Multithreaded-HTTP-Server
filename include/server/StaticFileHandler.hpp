#ifndef STATIC_FILE_HANDLER_HPP
#define STATIC_FILE_HANDLER_HPP

#include <string>

class StaticFileHandler {
public:
    std::string readFile(const std::string& path);
}