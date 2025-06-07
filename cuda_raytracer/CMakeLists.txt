cmake_minimum_required(VERSION 3.20)

# Set the project name and languages
project(cuda_raytracer LANGUAGES CXX CUDA)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_STANDARD_REQUIRED True)

# Set the CUDA architecture (e.g., 86 for RTX 30-series, 89 for 40-series)
# You can list multiple architectures, e.g., "75 86"
set(CMAKE_CUDA_ARCHITECTURES 89)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add the executable with all necessary .cu files
add_executable(cuda_raytracer
    src/main.cu
    src/color.cu
    src/hittable_list.cu
)

# Enable separable compilation for CUDA
set_target_properties(cuda_raytracer PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Link to the CUDA runtime library
target_link_libraries(cuda_raytracer PRIVATE cudart) 