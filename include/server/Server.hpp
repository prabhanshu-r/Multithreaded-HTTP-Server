#ifndef SERVER_HPP
#define SERVER_HPP


class Server {
public:
    Server();
    ~Server();

    void start();
private:
    int serverSocket;
};

#endif