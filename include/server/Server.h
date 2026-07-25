#ifndef SERVER_H
#define SERVER_H


class Server {
public:
    Server();
    ~Server();

    void start();
private:
    int serverSocket;
};

Server::Server() : serverSocket(-1) {}

#endif