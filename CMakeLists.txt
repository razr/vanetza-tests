cmake_minimum_required(VERSION 3.0)
project(vanetza-tests)

# Add the executable
add_executable(raw_socket_test raw_socket_test.cpp)
add_executable(raw_socket_hello raw_socket_hello.cpp)

# Specify the C standard
set(CMAKE_C_STANDARD 11)

# Add compiler flags if needed
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

