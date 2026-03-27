cmake_minimum_required(VERSION 3.16)
project(sim LANGUAGES C)

add_library(sim STATIC
    scheduler.c
    app.c
    sim_hal.c   # Qt-side HAL stub, or implement in C++
)

target_include_directories(sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sim PUBLIC trace_rtos)