cmake_minimum_required(VERSION 3.0)
project(vanetza-tests)

find_package(Boost REQUIRED COMPONENTS system)

# Add the executable
add_executable(raw_socket_bind raw_socket_bind.c)
add_executable(raw_socket_send_hello raw_socket_send_hello.c)
add_executable(raw_socket_ioctl raw_socket_ioctl.c)
add_executable(raw_socket_getsockname raw_socket_getsockname.c)
add_executable(boost_local_endpoint boost_local_endpoint.cpp)

# Specify the C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add compiler flags if needed
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Link against Boost libraries
target_link_libraries(boost_local_endpoint Boost::system)
