cmake_minimum_required(VERSION 3.14)

project(socket-wrapper)

set(SOURCES
    src/message.cpp
    src/tcpserver.cpp
    src/tcpsocket.cpp
    src/utils.cpp
    test/main.cpp
)

add_executable(socket-wrapper-test ${SOURCES})

target_include_directories(socket-wrapper-test
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(socket-wrapper-test PRIVATE -pg) # for use gprof
target_link_options(socket-wrapper-test PRIVATE -pg) # for use gprof

target_link_libraries(socket-wrapper-test pthread)
