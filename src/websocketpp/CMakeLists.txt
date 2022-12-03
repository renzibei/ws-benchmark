cmake_minimum_required(VERSION 3.13)
project(test_websocketpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(tpp_echo_server echo_server.cpp)



set(TPP_SOURCE_DIR ../../third_party/websocketpp)

add_library(tpp INTERFACE)
target_include_directories(tpp INTERFACE ${TPP_SOURCE_DIR})

target_link_libraries(tpp_echo_server PRIVATE tpp)





if(NOT MSVC)

    target_compile_options(
            tpp_echo_server
            PRIVATE
            -O3
            -Wall -Wextra
            #            -fsanitize=address
            -flto
    )

    target_link_options(tpp_echo_server
            PRIVATE
            -pthread
            )
endif()




target_link_options(tpp_echo_server
        PRIVATE
        #        -fsanitize=address
        -flto
        )