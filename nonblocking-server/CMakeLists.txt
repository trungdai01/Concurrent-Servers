cmake_minimum_required(VERSION 3.15)
project(NONBLOCKING_SERVER)

set(UTILS_ROOT "D:/Programming/C, C++/Concurrent Servers/utils")

add_executable(nonblocking-server nonblocking-listener.c)

target_link_libraries(nonblocking-server 
        "${UTILS_ROOT}/build/libutils_sv.a" 
        ws2_32
)
target_include_directories(nonblocking-server PUBLIC ${UTILS_ROOT})