#include "server/Socket.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>

Socket::~Socket() { close(); }

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

Socket Socket::listenOn(const std::string& host, int port, std::string& error) {
    Socket sock(::socket(AF_INET, SOCK_STREAM, 0));
    if (!sock.valid()) {
        error = std::string("socket: ") + std::strerror(errno);
        return Socket();
    }

    // Lets us restart the server right away instead of waiting for TIME_WAIT.
    int yes = 1;
    ::setsockopt(sock.fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        error = "invalid host address: " + host;
        return Socket();
    }

    if (::bind(sock.fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
        error = "bind to port " + std::to_string(port) + ": " + std::strerror(errno);
        return Socket();
    }
    if (::listen(sock.fd_, SOMAXCONN) == -1) {
        error = std::string("listen: ") + std::strerror(errno);
        return Socket();
    }
    return sock;
}

Socket Socket::acceptClient(std::string& clientIp) const {
    sockaddr_in address{};
    socklen_t length = sizeof(address);
    int fd = ::accept(fd_, reinterpret_cast<sockaddr*>(&address), &length);
    if (fd < 0) return Socket();

    char text[INET_ADDRSTRLEN] = {0};
    ::inet_ntop(AF_INET, &address.sin_addr, text, sizeof(text));
    clientIp = text;
    return Socket(fd);
}

bool Socket::setTimeouts(int seconds) const {
    timeval timeout{};
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return ::setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0 &&
           ::setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == 0;
}

ssize_t Socket::receive(char* buffer, std::size_t size) const {
    ssize_t n;
    do {
        n = ::recv(fd_, buffer, size, 0);
    } while (n < 0 && errno == EINTR);
    return n;
}

bool Socket::sendAll(const std::string& data) const {
    std::size_t sent = 0;
    while (sent < data.size()) {
        // MSG_NOSIGNAL: a client that hung up gives an error, not a SIGPIPE crash.
        ssize_t n = ::send(fd_, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        sent += static_cast<std::size_t>(n);
    }
    return true;
}

void Socket::drainAndClose() {
    if (!valid()) return;
    ::shutdown(fd_, SHUT_WR);  // client now sees end-of-response
    setTimeouts(1);            // never wait more than 1 s per read

    char scratch[4096];
    std::size_t total = 0;
    const std::size_t limit = 64 * 1024;  // never read more than 64 KB
    while (total < limit) {
        ssize_t n = ::recv(fd_, scratch, sizeof(scratch), 0);
        if (n <= 0) break;  // peer closed, timeout, or error
        total += static_cast<std::size_t>(n);
    }
    close();
}

void Socket::rejectAndClose(const std::string& response) {
    if (!valid()) return;

    // The client usually sends its request right after connecting, but it may not
    // have arrived yet. Give it up to 50 ms, then discard it: closing a socket
    // that still holds unread data makes TCP send a reset, which would wipe out
    // the reply before the client reads it. (Best effort; 50 ms is the most this
    // can ever stall the accept loop, and only while the server is overloaded.)
    pollfd waitFor{fd_, POLLIN, 0};
    ::poll(&waitFor, 1, 50);

    char scratch[4096];
    for (int i = 0; i < 16; ++i) {
        if (::recv(fd_, scratch, sizeof(scratch), MSG_DONTWAIT) <= 0) break;
    }
    sendAll(response);
    ::shutdown(fd_, SHUT_WR);
    close();
}
