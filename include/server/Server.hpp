#ifndef SERVER_HPP
#define SERVER_HPP

#include <atomic>
#include <string>

#include "server/Config.hpp"
#include "server/HttpParser.hpp"
#include "server/Router.hpp"
#include "server/Socket.hpp"
#include "server/StaticFileHandler.hpp"

class Server {
public:
    explicit Server(const Config& config);

    // Runs until requestStop() is called. Returns false if it could not start.
    bool start();

    // Safe to call from a signal handler (Ctrl+C / SIGTERM).
    static void requestStop();

private:
    void handleClient(Socket& client, const std::string& clientIp) const;

    // 0 = request read, -1 = client vanished (send nothing), else an HTTP error status.
    int readRequest(Socket& client, std::string& raw) const;

    static std::atomic<bool> stopRequested_;

    Config config_;
    StaticFileHandler files_;  // declared before router_: the router refers to it
    Router router_;
    HttpParser parser_;
};

#endif
