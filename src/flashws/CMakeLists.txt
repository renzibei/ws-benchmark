cmake_minimum_required(VERSION 3.13)
project(test_flashws)

set(CMAKE_CXX_STANDARD 17)

add_executable(fws_echo_client echo_client.cpp)
add_executable(fws_echo_server echo_server.cpp)

set(FLASHWS_SOURCE_DIR ../../third_party/flashws)
add_subdirectory(${FLASHWS_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/flashws)


target_link_libraries(fws_echo_client PRIVATE fws::flashws)
target_link_libraries(fws_echo_server PRIVATE fws::flashws)

OPTION(USE_RAW_HISTOGRAM "Use raw_histogram, otherwise use HdrHistogram_c" OFF)

if(USE_RAW_HISTOGRAM)
    ADD_DEFINITIONS(-DUSE_RAW_HISTOGRAM)
    SET(HDR_HIST_LIB "")
else()
#    get_target_property(FLASHWS_SOURCE_DIR fws::flashws SOURCE_DIR)

    add_subdirectory(${FLASHWS_SOURCE_DIR}/thirdparty/HdrHistogram_c ${CMAKE_CURRENT_BINARY_DIR}/HdrHistogram_c)
    SET(HDR_HIST_LIB hdr_histogram_static)
endif()

target_link_libraries(fws_echo_client
        PRIVATE ${HDR_HIST_LIB}
)



if(NOT MSVC)
    target_compile_options(
            fws_echo_client
            PRIVATE
            -O3
            #            -g
            -Wall -Wextra
            #            -fsanitize=address
    )

    target_compile_options(
            fws_echo_server
            PRIVATE
            -O3
            #            -g
            -Wall -Wextra
            #            -fsanitize=address
    )
endif()


target_link_options(fws_echo_client
        PRIVATE
        #        -fsanitize=address
        )



target_link_options(fws_echo_server
        PRIVATE
        #        -fsanitize=address

        )