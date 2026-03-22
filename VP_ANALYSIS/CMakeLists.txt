cmake_minimum_required(VERSION 3.20)
project(VP_ANALYSIS)

if(NOT DEFINED ENV{TRACE_DIR})
    message(FATAL_ERROR "environment variable TRACE_DIR not set ")
endif()

set(RUN_LOC "${CMAKE_SOURCE_DIR}/run/")
file(MAKE_DIRECTORY "${RUN_LOC}")

set(LOG_LOC ${CMAKE_SOURCE_DIR}/run/log/)
file(MAKE_DIRECTORY "${LOG_LOC}")

add_compile_definitions(TARGET_IA32E)
set(dirs ".")

# set(srcs)
# foreach(dir IN LISTS dirs)
#     file(
#         GLOB dir_srcs
#         "${dir}/*.hpp"
#         "${dir}/*.cpp"
#     )
#     set(srcs ${srcs} ${dir_srcs})
# endforeach()

file(
    GLOB srcs 
    "${dirs}/*.hpp"
    "${dirs}/*.cpp"
)

message(STATUS "srcs locs are: ${srcs}")

add_executable(main  ${srcs})

set(MATPLOTPP_BUILD_WITH_PEDANTIC_WARNINGS OFF)

add_subdirectory(matplot)
add_subdirectory(Zlib)

target_include_directories(main PUBLIC 
                "/opt/pin-3.31/extras/components/include"
                ${CMAKE_CURRENT_SOURCE_DIR}
            )

add_library(gzstream
            gzstream.h
            gzstream.C
)   
target_include_directories(gzstream PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR}/Zlib
                            )           
target_link_libraries(gzstream PUBLIC zlibstatic)

target_link_libraries(main PUBLIC  matplot gzstream )

target_compile_definitions(main PRIVATE TRACE_LOC="$ENV{TRACE_DIR}" LOG_LOC="${LOG_LOC}" RUN_LOC="${RUN_LOC}")

