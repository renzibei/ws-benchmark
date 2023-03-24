cmake_minimum_required(VERSION 3.13)
project(test_boost_beast)

# Set the required C++ standard
set(CMAKE_CXX_STANDARD 17)



# Find the Boost package and its required components (e.g., system and filesystem)
find_package(Boost 1.71 REQUIRED)

# Add your executable target (e.g., main.cpp)
#add_executable(${PROJECT_NAME} main.cpp)

# Link the Boost libraries to your target
#target_link_libraries(boost_echo_server Boost::beast Boost::asio)
include_directories({Boost_INCLUDE_DIRS})
add_executable(boost_echo_server echo_server.cpp)
target_link_options(boost_echo_server
        PRIVATE
        -pthread
        )