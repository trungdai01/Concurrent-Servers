cmake_minimum_required(VERSION 3.15)
project(SEQUENTIAL_SERVER)

set(UTILS_LIB "${PROJECT_SOURCE_DIR}/../utils/build/libutils_sv.a")
set(UTILS_DIR "${PROJECT_SOURCE_DIR}/../utils/")

add_executable(sequential-server sequential-server.c)

target_link_libraries(sequential-server ${UTILS_LIB} ws2_32)
target_include_directories(sequential-server PUBLIC ${UTILS_DIR})