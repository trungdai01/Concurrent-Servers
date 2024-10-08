cmake_minimum_required(VERSION 3.15)
project(UV_SERVER)

set(UTILS_ROOT "D:/Programming/C, C++/Concurrent Servers/utils")
set(LIBUV_ROOT "D:/Software/C Libraries/libuv-git/libuv")

# list(APPEND flags "-lpthread" "-pthread" "-Wall")
list(APPEND flags "-Wall")

add_executable(uv-server uv-server.c)

target_compile_options(uv-server
  PUBLIC
    ${flags}
)

target_link_libraries(uv-server 
          # "${UTILS_ROOT}/build/Debug/utils_sv.lib" 
          "${UTILS_ROOT}/build/libutils_sv.a" 
          # "${LIBUV_ROOT}/build/Release/libuv.lib"  
          "${LIBUV_ROOT}/build/libuv.a"  
      ws2_32
      pthread
      dbghelp
      psapi
      userenv
      shell32
      iphlpapi
  )
target_include_directories(uv-server PUBLIC ${UTILS_ROOT} "${LIBUV_ROOT}/include")