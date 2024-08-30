cmake_minimum_required(VERSION 3.15)
project(BLOCKING_SERVER)

set(UTILS_LIB "${PROJECT_SOURCE_DIR}/../utils/build/libutils_sv.a")
set(UTILS_DIR "${PROJECT_SOURCE_DIR}/../utils/")

add_executable(blocking-server blocking-listener.c)

target_link_libraries(blocking-server ${UTILS_LIB} ws2_32)
target_include_directories(blocking-server PUBLIC ${UTILS_DIR})