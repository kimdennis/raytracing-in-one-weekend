cmake_minimum_required(VERSION 3.10)

# Set the project name
project(RayTracingInOneWeekend)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add the executable
add_executable(raytracing src/main.cpp )
