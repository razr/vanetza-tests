cmake_minimum_required(VERSION 3.0)
project(vanetza-tests)

find_package(Boost REQUIRED COMPONENTS system)

# Add the executable
add_executable(raw_socket_test raw_socket_test.c)
add_executable(raw_socket_hello raw_socket_hello.c)
add_executable(local_endpoint_test local_endpoint_test.cpp)
add_executable(local_endpoint_c_test local_endpoint_c_test.c)

# Specify the C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add compiler flags if needed
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Link against Boost libraries
target_link_libraries(local_endpoint_test Boost::system)
