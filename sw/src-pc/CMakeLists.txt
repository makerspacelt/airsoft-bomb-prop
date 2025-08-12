# build using:
# mkdir -p build && cd build
# cmake ..
# make

cmake_minimum_required(VERSION 3.10)

project(ant)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3")

set(SRCS
    ../src-common/utilities.cpp
    main.cpp
)

add_executable(ant ${SRCS})
