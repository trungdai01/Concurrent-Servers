cmake_minimum_required(VERSION 3.15)
project(UV_TIMER_THREADS)

set(LIBUV_ROOT "D:/Software/C Libraries/libuv-git/libuv")

list(APPEND flags "-Wall")

add_executable(uv-timer-threads uv-timer-threads.c)

target_compile_options(uv-timer-threads
  PUBLIC
    ${flags}
)

target_link_libraries(uv-timer-threads 
          "${LIBUV_ROOT}/build/libuv.a"  
      ws2_32
      pthread
      dbghelp
      psapi
      userenv
      shell32
      iphlpapi
  )
target_include_directories(uv-timer-threads PUBLIC "${LIBUV_ROOT}/include")