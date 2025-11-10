cmake_minimum_required(VERSION 4.0)
project(chat_server)

set(CMAKE_CXX_STANDARD 23)

add_executable(client src/client.cpp)
add_executable(server src/server.cpp)
