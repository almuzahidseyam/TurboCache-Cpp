#include "Server.h"
#include <iostream>
#include <sstream>
#include <vector>

Server::Server(int p, size_t numThreads) 
    : port(p), threadPool(numThreads), isRunning(false), serverSocket(INVALID_SOCKET) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed.");
    }
}

Server::~Server() {
    stop();
    WSACleanup();
}

void Server::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        throw std::runtime_error("Socket creation failed.");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        throw std::runtime_error("Bind failed.");
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(serverSocket);
        throw std::runtime_error("Listen failed.");
    }

    isRunning = true;
    std::cout << "[INFO] TurboCache Server listening on port " << port << "...\n";

    while (isRunning) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            if (isRunning) std::cerr << "[ERROR] Accept failed.\n";
            continue;
        }

        // Pass client handling to Thread Pool
        threadPool.enqueue([this, clientSocket]() {
            this->handleClient(clientSocket);
        });
    }
}

void Server::stop() {
    isRunning = false;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
}

void Server::handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) break; // Client disconnected or error
        
        buffer[bytesReceived] = '\0';
        std::string request(buffer);
        
        // Process standard RESP-like strings
        std::string response = processCommand(request);
        
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    closesocket(clientSocket);
}

std::string Server::processCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    // Convert to uppercase for basic case insensitivity
    for (auto & c: cmd) c = toupper(c);

    if (cmd == "PING") {
        return "PONG\r\n";
    } 
    else if (cmd == "SET") {
        std::string key, value;
        iss >> key >> value;
        if (key.empty() || value.empty()) return "-ERR syntax error\r\n";
        cache.set(key, value);
        return "+OK\r\n";
    } 
    else if (cmd == "GET") {
        std::string key;
        iss >> key;
        if (key.empty()) return "-ERR syntax error\r\n";
        auto optVal = cache.get(key);
        if (optVal) {
            return "$" + std::to_string(optVal->length()) + "\r\n" + *optVal + "\r\n";
        } else {
            return "$-1\r\n"; // Null reply
        }
    } 
    else if (cmd == "DEL") {
        std::string key;
        iss >> key;
        if (key.empty()) return "-ERR syntax error\r\n";
        bool deleted = cache.del(key);
        return deleted ? ":1\r\n" : ":0\r\n";
    }
    else if (cmd == "DBSIZE") {
        return ":" + std::to_string(cache.size()) + "\r\n";
    }
    
    return "-ERR unknown command '" + cmd + "'\r\n";
}
