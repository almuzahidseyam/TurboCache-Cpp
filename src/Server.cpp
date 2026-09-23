#include "Server.h"
#include <iostream>
#include <sstream>
#include <vector>

Server::Server(int p, size_t numThreads) 
    : port(p), threadPool(numThreads), serverSocket(INVALID_SOCKET), isRunning(false) {
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

// A recv() returns whatever slice of the stream has arrived: it may hold two
// commands, or half of one. The previous version treated every read as exactly
// one command, so a pipelined "SET a 1\r\nSET b 2\r\n" stored only the first and
// discarded the rest without an error, and a command split across two segments
// was stored truncated. Each connection now keeps its own buffer and consumes
// whole lines, leaving any partial tail for the next read.
void Server::handleClient(SOCKET clientSocket) {
    char chunk[4096];
    std::string pending;
    // A client that never sends a newline must not be able to grow this without
    // bound; 64 KiB is far beyond any legitimate single command.
    const size_t MAX_PENDING = 64 * 1024;

    while (true) {
        int bytesReceived = recv(clientSocket, chunk, sizeof(chunk), 0);
        if (bytesReceived <= 0) break; // Client disconnected or error

        pending.append(chunk, static_cast<size_t>(bytesReceived));

        size_t newline;
        while ((newline = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, newline);
            pending.erase(0, newline + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::string response = processCommand(line);
            if (send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0) == SOCKET_ERROR) {
                closesocket(clientSocket);
                return;
            }
        }

        if (pending.size() > MAX_PENDING) {
            const std::string tooLong = "-ERR command too long\r\n";
            send(clientSocket, tooLong.c_str(), static_cast<int>(tooLong.length()), 0);
            break;
        }
    }
    closesocket(clientSocket);
}

std::string Server::processCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    // Convert to uppercase for basic case insensitivity
    for (auto & c: cmd) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));

    if (cmd == "PING") {
        // Was "PONG\r\n". Without the + it is not a simple string, not a bulk
        // string, not an integer and not an error, so a real RESP client cannot
        // parse it -- telnet hid this because a human reads it fine either way.
        return "+PONG\r\n";
    } 
    else if (cmd == "SET") {
        std::string key;
        iss >> key;
        // `iss >> value` read a single whitespace-delimited token, so
        // `SET name Muhammad Al-Muzahid` stored "Muhammad" and still answered
        // +OK. Silent truncation with a success reply is worse than an error,
        // so the value is now everything after the key.
        std::string value;
        std::getline(iss, value);
        size_t firstNonSpace = value.find_first_not_of(" \t");
        value = (firstNonSpace == std::string::npos) ? "" : value.substr(firstNonSpace);
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
