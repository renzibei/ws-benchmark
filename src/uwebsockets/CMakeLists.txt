cmake_minimum_required(VERSION 3.13)
project(test_uwebsockets)

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

add_library(u_sockets ${u_sockets_src})
#include_directories(${u_socket})
#target_include_directories(u_sockets
#        PUBLIC
#        ${u_sockets_hdr}
#        )
#
#target_compile_definitions(u_sockets
#        PRIVATE
#        -DLIBUS_USE_OPENSSL
##        -DLIBUS_NO_SSL
#        )
#
#target_compile_options(u_sockets
#        PRIVATE
#        -std=c11
#)

target_include_directories(u_sockets
        PUBLIC
        ${u_sockets_hdr}
)

target_compile_definitions(u_sockets
        PRIVATE
        -DLIBUS_USE_OPENSSL
)

target_link_libraries(u_sockets
        PRIVATE
        OpenSSL::SSL
)
set_target_properties(u_sockets
        PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 20

)


add_library(uws INTERFACE)
target_include_directories(uws INTERFACE ${U_WS_DIR}/src)
target_compile_definitions(uws
        INTERFACE
        -DUWS_NO_ZLIB
        )

target_compile_options(uws
        INTERFACE
        -flto
        )


target_link_libraries(uws
        INTERFACE
        u_sockets
)

add_executable(uws_echo_server echo_server.cpp)

target_link_libraries(uws_echo_server
        PRIVATE
        uws
        )

target_link_options(uws_echo_server
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