# Ray Tracing in One Weekend

This project is a C++ and CUDA implementation of the **Ray Tracing in One Weekend** tutorial by Peter Shirley. It renders a 3D scene with spheres using ray tracing techniques, producing an image through light simulation and ray-object interactions.

## Features

* **Multiple Implementations**:
  - CPU version in C++
  - GPU-accelerated version using CUDA
* **Ray-Sphere Intersection**: Computes intersections between rays and spheres
* **Materials**: Supports Lambertian (diffuse), metallic, and dielectric (glass) surfaces
* **Random Sampling**: Used for anti-aliasing and producing realistic lighting
* **Multisampling**: Averages multiple rays per pixel for smooth and high-quality image output
* **Output**: Renders an image in the PPM format, which can be converted to PNG or other image formats

## Requirements

### CPU Version
* **C++17 or later**
* **CMake** (for building the project)
* **ImageMagick** (for converting the PPM image to PNG or other formats)

### CUDA Version
* **CUDA Toolkit** (tested with CUDA 12.x)
* **NVIDIA GPU** with CUDA support
* **ImageMagick** (for converting the PPM image to PNG or other formats)

## Installation

1. **Clone the repository**:  
```bash
git clone https://github.com/kimdennis/raytracing-in-one-weekend.git  
cd raytracing-in-one-weekend
```

2. **Build the CPU version using CMake**:  
```bash
mkdir build  
cd build  
cmake ..  
cmake --build .
```

3. **Build the CUDA version**:  
```bash
cd cuda_raytracer
nvcc src/main.cu -o raytracer -I src/ --extended-lambda
```

## Usage

### CPU Version
After compiling, run the CPU version:
```bash
./RayTracer > image.ppm
```

### CUDA Version
Run the CUDA version:
```bash
./raytracer
```

The program will render an image of a scene containing randomly positioned spheres. The default output image is saved as `image.ppm` (CPU version) or `cuda_image.ppm` (CUDA version) in the current directory.

To view or convert the `.ppm` file to `.png` or any other format, use ImageMagick:
```bash
magick image.ppm image.png
# or for CUDA version
magick cuda_image.ppm cuda_image.png
```

## Performance

The CUDA implementation provides significant speedup compared to the CPU version:
- Uses parallel processing on the GPU
- Optimized block and grid sizes for better performance
- Configurable quality parameters:
  - Image resolution
  - Samples per pixel
  - Maximum ray depth
  - Block size for CUDA kernels

## About

This project is based on the "Ray Tracing in One Weekend" book by Peter Shirley, with added CUDA implementation for GPU acceleration. The CUDA version maintains the same visual quality as the CPU version while providing better performance through parallel processing.

## License

This project is open source and available under the MIT License.
