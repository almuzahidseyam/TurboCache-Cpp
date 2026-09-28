#pragma once
#include "Cache.h"
#include "ThreadPool.h"
#include <string>

// Cross-Platform Socket Definitions
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    
    typedef int SOCKET;
    const int INVALID_SOCKET = -1;
    const int SOCKET_ERROR = -1;
    #define closesocket(s) close(s)
#endif

class Server {
private:
    int port;
    Cache cache;
    ThreadPool threadPool;
    // Declaration order is the initialisation order, whatever the constructor's
    // list says, so these two are ordered to agree with it.
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
