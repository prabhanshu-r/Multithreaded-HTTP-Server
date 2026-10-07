#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cstddef>
#include <string>
#include <sys/types.h>

// Owns a socket file descriptor and closes it automatically (RAII).
// Move-only, so a descriptor can never be closed twice or leaked.
class Socket {
public:
    Socket() = default;
    explicit Socket(int fd) : fd_(fd) {}
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool valid() const { return fd_ >= 0; }
    int fd() const { return fd_; }
    void close();

    // Creates a socket that is bound and listening. On failure the result is
    // invalid and `error` explains why.
    static Socket listenOn(const std::string& host, int port, std::string& error);

    // Accepts one client. Returns an invalid Socket on failure.
    Socket acceptClient(std::string& clientIp) const;

    // Limits how long a single recv()/send() may block.
    bool setTimeouts(int seconds) const;

    // Returns bytes read, 0 if the peer closed, -1 on error/timeout (see errno).
    ssize_t receive(char* buffer, std::size_t size) const;

    // Sends everything, looping over partial writes.
    bool sendAll(const std::string& data) const;

    // Polite close: tell the peer we are done sending, read and discard whatever
    // it still sends (bounded in size and time), then close. Closing a socket
    // that still has unread data makes TCP send a reset, which can destroy the
    // response the client has not read yet.
    void drainAndClose();

    // Like drainAndClose() but never waits: used on the accept thread to turn a
    // client away ("503") without stalling new connections. Best effort.
    void rejectAndClose(const std::string& response);

private:
    int fd_ = -1;
};

#endif
