cmake_minimum_required(VERSION 3.15)
project(BLOCKING_SERVER)

set(UTILS_ROOT "D:/Programming/C, C++/Concurrent Servers/utils")

add_executable(blocking-server blocking-listener.c)

target_link_libraries(blocking-server 
        "${UTILS_ROOT}/build/libutils_sv.a" 
        ws2_32
)
target_include_directories(blocking-server PUBLIC ${UTILS_ROOT})