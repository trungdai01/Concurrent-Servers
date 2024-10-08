cmake_minimum_required(VERSION 3.15)
project(SELECT_SERVER)

set(UTILS_ROOT "D:/Programming/C, C++/Concurrent Servers/utils")

add_executable(select-server select-server.c)

target_link_libraries(select-server 
        "${UTILS_ROOT}/build/libutils_sv.a"
        ws2_32
)
target_include_directories(select-server PUBLIC ${UTILS_ROOT})