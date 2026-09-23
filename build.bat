@echo off
echo Compiling TurboCache-Cpp...
g++ src\main.cpp src\Server.cpp src\Cache.cpp -o TurboCache.exe -lws2_32 -std=c++17 -O3 -pthread
if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b %errorlevel%
)
echo Compilation successful. Run TurboCache.exe to start the server.
