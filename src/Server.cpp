#include "server/Server.hpp"
#include "server/HttpParser.hpp"
#include "server/HttpRequest.hpp"
#include "server/Router.hpp"
#include "server/HttpResponse.hpp"

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


Server::Server() : serverSocket(-1){}

Server::~Server(){
    if(serverSocket >= 0) close(serverSocket);
}

void Server::start() {
    // create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        std::cerr << "Failed to create socket.\n";
        return;
    }

    std::cout <<"Socket Created\n";

    //configure address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //Bind
    int bid  = bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    if(bid == -1) {
        std::cerr << "Bind failed.\n";
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    std::cout << "Bind Successful\n";

    //Listen
    if(listen(serverSocket, 5) == -1) {
        std::cerr << "Listen failed.\n";
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    std::cout << "Listening on port 8080...\n";
    std::cout << "Waiting for clint...\n";

    //Accept
    sockaddr_in clientAddress{};
    socklen_t clientLenght = sizeof(clientAddress);

    int clientSocket = accept(
        serverSocket,
        reinterpret_cast<sockaddr*>(&clientAddress),
        &clientLenght
    );

    if(clientSocket == -1) {
        std::cerr << "Accept failed.\n";
        return;
    }

    std::cout << "Client Connected!\n";
    std::cout << "Client Socket FD : " << clientSocket << '\n';

    //Receive

    char buffer[4096] = {0};

    ssize_t bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer) -1,
        0
    );

    if (bytesReceived == -1) {
        std:: cerr << "Receive failed.\n";
        close(clientSocket);
        return;
    }    

    buffer[bytesReceived] = '\0';

    std::cout << "\nHTTP REQUEST\n";
    
    HttpParser parser;

    HttpRequest request = parser.parse(buffer);

    // std::cout << "Parsed Request\n";

    // std::cout << "Method : " << request.method<< "\n";
    // std::cout << "path : " << request.path<< "\n";
    // std::cout << "version : " << request.version<< "\n";
    // //Send

    // std::string body = "<html><h1>Hello from C++ server!</h1></html>";

    // std::string response =
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-Type: text/html\r\n"
    //     "Content-Length: " +
    //     std::to_string(body.size()) +
    //     "\r\n\r\n" +
    //     body;

    Router router;

    HttpResponse response = router.route(request);

    std::string httpResponse = response.toString();

    send(
        clientSocket,
        httpResponse.c_str(),
        httpResponse.size(),
        0
    );

    close(clientSocket);
}