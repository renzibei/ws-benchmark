cmake_minimum_required(VERSION 3.13)
project(test_usockets_tcp)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

set(U_WS_DIR "../../third_party/uWebSockets")
set(U_SOCKETS_DIR  "${U_WS_DIR}/uSockets")

file(GLOB u_sockets_src
        "${U_SOCKETS_DIR}/src/*.c"
        "${U_SOCKETS_DIR}/src/crypto/*.c"
        "${U_SOCKETS_DIR}/src/crypto/*.cpp"
        "${U_SOCKETS_DIR}/src/eventing/*.c"
        )

file(GLOB u_sockets_hdr
        "${U_SOCKETS_DIR}/src"
        )

add_library(us_sockets ${u_sockets_src})
#include_directories(${u_socket})
target_include_directories(us_sockets
        PUBLIC
        ${u_sockets_hdr}
        )

target_compile_definitions(us_sockets
        PRIVATE
        -DLIBUS_USE_OPENSSL

        )

target_link_libraries(us_sockets
        PRIVATE
        OpenSSL::SSL
)

#target_compile_options(us_sockets
#        PRIVATE
#        -std=c11
#)
set_target_properties(us_sockets
        PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 20

)

add_executable(us_echo_server echo_server.cpp)

target_link_libraries(us_echo_server
        PRIVATE
        us_sockets
        )

target_link_options(us_echo_server
        PRIVATE
        -flto
        )

add_executable(us_https_client https_client.cpp)

target_link_libraries(us_https_client
        PRIVATE
        us_sockets
        )

target_link_options(us_https_client
        PRIVATE
        -flto
)

#add_executable(echo_client echo_client.cpp)
#target_link_libraries(echo_client
#        PRIVATE
#        uws
#        )
#
#target_link_options(echo_client
#        PRIVATE
#        -flto
#        )