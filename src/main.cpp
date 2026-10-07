#include <csignal>
#include <cstdlib>
#include <string>

#include "server/Config.hpp"
#include "server/Logger.hpp"
#include "server/Server.hpp"

namespace {
void onSignal(int) { Server::requestStop(); }
}  // namespace

int main(int argc, char* argv[]) {
    const std::string configPath = (argc > 1) ? argv[1] : "config/server.conf";

    Config config;
    if (!config.load(configPath)) {
        Logger::warn("could not read '" + configPath + "', using defaults");
    }
    config.applyEnvironment();

    // Ctrl+C or `docker stop` -> finish current requests, then exit cleanly.
    struct sigaction action{};
    action.sa_handler = onSignal;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);
    std::signal(SIGPIPE, SIG_IGN);

    Server server(config);
    return server.start() ? 0 : 1;
}
