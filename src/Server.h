#pragma once
#include "Cache.h"
#include "ThreadPool.h"
#include <string>

// Need Winsock2 for Windows networking
#include <winsock2.h>
#include <ws2tcpip.h>

class Server {
private:
    int port;
    Cache cache;
    ThreadPool threadPool;
    SOCKET serverSocket;
    bool isRunning;

    void handleClient(SOCKET clientSocket);
    std::string processCommand(const std::string& input);

public:
    Server(int port, size_t numThreads);
    ~Server();
    void start();
    void stop();
};
