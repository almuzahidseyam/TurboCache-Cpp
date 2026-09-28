cmake_minimum_required(VERSION 3.14)
project(TurboCache CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If threading is used
find_package(Threads REQUIRED)

# ---------------------------------------------------------
# 1. Main Library (Core logic)
# ---------------------------------------------------------
add_library(TurboCache_Core
    src/Cache.cpp
    src/Server.cpp
)
target_include_directories(TurboCache_Core PUBLIC src)
target_link_libraries(TurboCache_Core PUBLIC Threads::Threads)
# We might need to link winsock32 on Windows for networking inside Server.cpp
if(WIN32)
    target_link_libraries(TurboCache_Core PUBLIC ws2_32)
endif()

# ---------------------------------------------------------
# 2. Main Executable
# ---------------------------------------------------------
add_executable(TurboCache_Server src/main.cpp)
target_link_libraries(TurboCache_Server PRIVATE TurboCache_Core)

# ---------------------------------------------------------
# 3. Google Test
# ---------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ---------------------------------------------------------
# 4. Unit Tests
# ---------------------------------------------------------
add_executable(TurboCache_Tests
    tests/test_cache.cpp
)
target_link_libraries(TurboCache_Tests PRIVATE 
    TurboCache_Core
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(TurboCache_Tests)
