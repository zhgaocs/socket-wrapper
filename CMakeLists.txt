cmake_minimum_required(VERSION 3.14)

project(socket-wrapper)

set(SOURCES
    src/tcpclient.cpp
    src/tcpserver.cpp
    test/main.cpp
)

add_executable(socket-wrapper-test ${SOURCES})

target_include_directories(socket-wrapper-test
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(socket-wrapper-test pthread)