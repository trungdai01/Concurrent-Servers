cmake_minimum_required(VERSION 3.15)
project(SELECT_SERVER)

set(UTILS_LIB "${PROJECT_SOURCE_DIR}/../utils/build/libutils_sv.a")
set(UTILS_DIR "${PROJECT_SOURCE_DIR}/../utils/")

add_executable(select-server select-server.c)

target_link_libraries(select-server ${UTILS_LIB} ws2_32)
target_include_directories(select-server PUBLIC ${UTILS_DIR})