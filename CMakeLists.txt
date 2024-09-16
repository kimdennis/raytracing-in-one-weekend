cmake_minimum_required(VERSION 3.10)

# Set the project name
project(RayTracingInOneWeekend)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable and include all source files
add_executable(raytracing src/main.cpp  )
