cmake_minimum_required(VERSION 3.13)
project(test_uwebsockets)

add_subdirectory(src/flashws)
add_subdirectory(src/uwebsockets)
add_subdirectory(src/websocketpp)
add_subdirectory(src/usockets-tcp)
add_subdirectory(src/boost-beast)