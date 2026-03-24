cmake_minimum_required(VERSION 3.16)

project(trace_rtos LANGUAGES C)

add_library(trace_rtos STATIC
    trace_rtos.c
)

target_include_directories(trace_rtos PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
