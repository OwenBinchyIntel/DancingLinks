cmake_minimum_required(VERSION 3.14)
project(DancingLinks VERSION 0.1.0 LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find all .cpp files in this folder
file(GLOB SOURCES "*.cpp")

# Create one executable per .cpp file
foreach(source ${SOURCES})
    get_filename_component(exe_name ${source} NAME_WE) # NAME without extension
    add_executable(${exe_name} ${source})
endforeach()

# Enable testing if needed
include(CTest)
enable_testing()
