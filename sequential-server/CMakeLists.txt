cmake_minimum_required(VERSION 3.15)
project(SEQUENTIAL_SERVER)

set(UTILS_ROOT "D:/Programming/C, C++/Concurrent Servers/utils")

add_executable(sequential-server sequential-server.c)

target_link_libraries(sequential-server 
        "${UTILS_ROOT}/build/libutils_sv.a"
        ws2_32
)
target_include_directories(sequential-server PUBLIC ${UTILS_ROOT})