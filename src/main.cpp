#include "Server.h"
#include <iostream>

int main() {
    try {
        std::cout << "Starting TurboCache-Cpp (Mini Redis)\n";
        std::cout << "Initializing with 4 worker threads...\n";
        
        // Start server on port 6379 (Redis default) with 4 threads
        Server server(6379, 4);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
