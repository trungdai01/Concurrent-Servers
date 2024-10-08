cmake_minimum_required(VERSION 3.10)

project(ThreadedServer LANGUAGES C)

set(UTILS_ROOT "D:/Programming/C, C++/Concurrent Servers/utils")

list(APPEND flags "-lpthread" "-pthread")

add_executable(threaded-server threaded-server.c)

target_compile_options(threaded-server
    PRIVATE
        ${flags}
)

target_link_libraries(threaded-server 
                "${UTILS_ROOT}/build/libutils_sv.a" 
                ws2_32
)
target_include_directories(threaded-server 
            PUBLIC 
                ${UTILS_ROOT} 
)