cmake_minimum_required(VERSION 3.5)

project(server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "-pthread -lboost_system")

add_executable(server session.h session.cpp tools.h tcp_server.cpp tcp_server.h main.cpp)
