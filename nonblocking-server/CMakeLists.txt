cmake_minimum_required(VERSION 3.15)
project(NONBLOCKING_SERVER)

set(UTILS_LIB "${PROJECT_SOURCE_DIR}/../utils/build/libutils_sv.a")
set(UTILS_DIR "${PROJECT_SOURCE_DIR}/../utils/")

add_executable(nonblocking-server nonblocking-listener.c)

target_link_libraries(nonblocking-server ${UTILS_LIB} ws2_32)
target_include_directories(nonblocking-server PUBLIC ${UTILS_DIR})