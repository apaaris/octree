cmake_minimum_required(VERSION 3.10)

project(Octree)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create out directory for VTK exports
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/out)

add_executable(octree src/main.cpp)

target_include_directories(octree PRIVATE src)



