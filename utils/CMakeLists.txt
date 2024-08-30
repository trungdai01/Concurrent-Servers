cmake_minimum_required(VERSION 3.10)
project(Utils LANGUAGES C)

add_library(utils_sv 
    STATIC
        utils.c
    )

target_include_directories(utils_sv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})