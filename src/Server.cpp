#include "server/Server.hpp"

#include <poll.h>
#include <pthread.h>
#include <csignal>

#include <cerrno>
#include <chrono>
#include <exception>
#include <memory>

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"
#include "server/Logger.hpp"
#include "server/ThreadPool.hpp"

std::atomic<bool> Server::stopRequested_{false};
static_assert(std::atomic<bool>::is_always_lock_free, "needed to be signal-safe");

Server::Server(const Config& config)
    : config_(config), files_(config.documentRoot), router_(files_) {}

void Server::requestStop() { stopRequested_.store(true); }

bool Server::start() {
    std::string error;
    Socket listener = Socket::listenOn(config_.host, config_.port, error);
    if (!listener.valid()) {
        Logger::error("cannot start: " + error);
        return false;
    }

    Logger::info("listening on " + config_.host + ":" + std::to_string(config_.port) + " with " +
                 std::to_string(config_.workers) + " workers, serving '" + config_.documentRoot + "'");

    // Block Ctrl+C/SIGTERM while the workers start. Threads inherit the blocked
    // mask, so these signals can only interrupt THIS thread's poll() and a stop
    // request is noticed immediately instead of after the poll timeout.
    sigset_t stopSignals, previousMask;
    sigemptyset(&stopSignals);
    sigaddset(&stopSignals, SIGINT);
    sigaddset(&stopSignals, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &stopSignals, &previousMask);
    ThreadPool pool(config_.workers, config_.queueLimit);
    pthread_sigmask(SIG_SETMASK, &previousMask, nullptr);

    // The main thread only accepts connections; the workers do all the real work.
    while (!stopRequested_.load()) {
        // Wake up every 500 ms so we notice a stop request even when idle.
        pollfd waitFor{listener.fd(), POLLIN, 0};
        int ready = ::poll(&waitFor, 1, 500);
        if (ready < 0) {
            if (errno == EINTR) continue;  // a signal arrived; re-check the stop flag
            Logger::error("poll failed");
            break;
        }
        if (ready == 0) continue;

        std::string clientIp;
        Socket client = listener.acceptClient(clientIp);
        if (!client.valid()) continue;
        client.setTimeouts(config_.readTimeoutSeconds);

        // std::function needs a copyable callable, so share the move-only Socket.
        auto shared = std::make_shared<Socket>(std::move(client));
        bool queued = pool.submit([this, shared, clientIp] { handleClient(*shared, clientIp); });

        if (!queued) {
            Logger::warn("queue full, rejecting " + clientIp);
            shared->rejectAndClose(HttpResponse::error(503).serialize());
        }
    }

    Logger::info("shutting down, finishing in-flight requests...");
    pool.shutdown();
    Logger::info("stopped");
    return true;
}

int Server::readRequest(Socket& client, std::string& raw) const {
    using Clock = std::chrono::steady_clock;
    // SO_RCVTIMEO limits ONE recv(); this deadline limits the whole request, so a
    // client dripping one byte every few seconds cannot hold a worker forever.
    const auto deadline = Clock::now() + std::chrono::seconds(config_.readTimeoutSeconds);

    char buffer[2048];
    while (raw.find("\r\n\r\n") == std::string::npos) {
        if (raw.size() > config_.maxRequestSize) return 431;
        if (Clock::now() > deadline) return 408;

        ssize_t n = client.receive(buffer, sizeof(buffer));
        if (n == 0) return -1;  // client closed the connection
        if (n < 0) return (errno == EAGAIN || errno == EWOULDBLOCK) ? 408 : -1;
        raw.append(buffer, static_cast<std::size_t>(n));
    }
    if (raw.find("\r\n\r\n") > config_.maxRequestSize) return 431;
    return 0;
}

void Server::handleClient(Socket& client, const std::string& clientIp) const {
    HttpResponse response;
    std::string summary = "-";
    bool headOnly = false;

    try {
        std::string raw;
        int readStatus = readRequest(client, raw);
        if (readStatus < 0) return;

        if (readStatus > 0) {
            response = HttpResponse::error(readStatus);
        } else {
            HttpRequest request;
            int parseError = 0;
            if (!parser_.parse(raw, request, parseError)) {
                response = HttpResponse::error(parseError);
            } else {
                response = router_.route(request);
                summary = request.method + " " + request.path;
                headOnly = (request.method == "HEAD");
            }
        }
    } catch (const std::exception& e) {
        Logger::error(std::string("request failed: ") + e.what());
        response = HttpResponse::error(500);
    }

    client.sendAll(response.serialize(!headOnly));
    client.drainAndClose();
    Logger::info(clientIp + " " + summary + " -> " + std::to_string(response.statusCode));
}
