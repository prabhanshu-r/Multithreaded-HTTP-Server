#include <server/StaticFileHandler.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

std::string StaticFileHandler::readFile(const std::string& path) {
    std::ifstream file(path);

    if(!file.is_open()) return "";

    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}