cmake_minimum_required(VERSION 3.15)
project(UV_TIMER_SLEEP)

set(LIBUV_ROOT "D:/Software/C Libraries/libuv-git/libuv")

list(APPEND flags "-Wall")

add_executable(uv-timer-sleep uv-timer-sleep.c)

target_compile_options(uv-timer-sleep
  PUBLIC
    ${flags}
)

target_link_libraries(uv-timer-sleep 
          "${LIBUV_ROOT}/build/libuv.a"  
      ws2_32
      pthread
      dbghelp
      psapi
      userenv
      shell32
      iphlpapi
  )
target_include_directories(uv-timer-sleep PUBLIC "${LIBUV_ROOT}/include")