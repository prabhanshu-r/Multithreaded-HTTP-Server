#include "server/Server.hpp"
#include "server/StaticFileHandler.hpp"
#include <iostream>

int main() {
    Server server;
    server.start();

    StaticFileHandler fileHandler;

    std::cout << fileHandler.readFile("public/index.html");

    return 0;
}