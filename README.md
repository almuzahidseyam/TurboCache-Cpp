# TurboCache-Cpp ??

A high-performance, multi-threaded, in-memory caching system (like a mini Redis) built entirely in raw C++17 from scratch.
Designed to handle thousands of concurrent TCP connections efficiently using a custom Thread Pool and thread-safe data structures.

## Features
- **Raw TCP Server:** Built using Windows Sockets API (Winsock2). No bloated third-party frameworks.
- **Thread Pool Architecture:** Reuses worker threads to handle client connections without the overhead of thread creation per request.
- **Thread-Safe Key-Value Store:** Implements `std::shared_mutex` (Readers-Writer lock) to allow multiple simultaneous reads, but exclusive writes.
- **RESP-like Protocol Support:** Communicates with simple text protocols (`SET`, `GET`, `DEL`, `DBSIZE`, `PING`).

## How to Build and Run
1. Ensure you have `g++` installed (MinGW on Windows).
2. Run `build.bat` in the root directory.
3. Start the server by running `TurboCache.exe`. The server listens on port `6379`.

## Usage
You can connect to the server using standard `telnet` or `netcat`:
```
> telnet localhost 6379

SET name Seyam
+OK

GET name
$5
Seyam

DBSIZE
:1

DEL name
:1
```
