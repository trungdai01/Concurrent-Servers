cmake_minimum_required(VERSION 3.10)

project(ThreadedServer LANGUAGES C)

set(UTILS_DIR "${PROJECT_SOURCE_DIR}/../utils/")
set(UTILS_LIB "${PROJECT_SOURCE_DIR}/../utils/build/libutils_sv.a")

link_directories(${UTILS_DIR}/build/)
# include_directories(${UTILS_DIR})

list(APPEND flags "-lpthread")

add_executable(threaded-server threaded-server.c)

target_compile_options(threaded-server
    PRIVATE
        ${flags}
)
target_link_libraries(threaded-server ${UTILS_LIB} ws2_32)
target_include_directories(threaded-server PUBLIC ${UTILS_DIR})