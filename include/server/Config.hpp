#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef>
#include <string>

// Server settings. The defaults are used for anything the config file omits.
struct Config {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::size_t workers = 4;
    std::size_t queueLimit = 128;
    std::string documentRoot = "public";
    std::size_t maxRequestSize = 8192;
    int readTimeoutSeconds = 5;

    // Reads "key = value" lines. Returns false if the file cannot be opened.
    bool load(const std::string& filename);

    // Lets the PORT environment variable override the port (used by cloud hosts).
    void applyEnvironment();
};

#endif
