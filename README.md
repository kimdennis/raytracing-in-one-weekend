# Ray Tracing in One Weekend

This project is a C++ implementation of the **Ray Tracing in One Weekend** tutorial by Peter Shirley. It renders a 3D scene with spheres using ray tracing techniques, producing an image through light simulation and ray-object interactions.

## Features

- **Ray-Sphere Intersection**: Computes intersections between rays and spheres.
- **Materials**: Supports Lambertian (diffuse) and metallic surfaces.
- **Random Sampling**: Used for anti-aliasing and producing realistic lighting.
- **Multisampling**: Averages multiple rays per pixel for smooth and high-quality image output.
- **Output**: Renders an image in the PPM format, which can be converted to PNG or other image formats.

## Requirements

- **C++17 or later**
- **CMake** (for building the project)
- **ImageMagick** (for converting the PPM image to PNG or other formats)

## Installation

1. **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/raytracing-in-one-weekend.git
    cd raytracing-in-one-weekend
    ```

2. **Build the project using CMake**:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

3. **Run the executable**:
    ```bash
    ./RayTracer > image.ppm
    ```

4. **Convert the PPM file to PNG using ImageMagick**:
    ```bash
    magick image.ppm image.png
    ```

## Usage

After compiling, the program will render an image of a scene containing randomly positioned spheres. The default output image is saved as `image.ppm` in the current directory.

To view or convert the `.ppm` file to `.png` or any other format, use tools like **ImageMagick**:
```bash
magick image.ppm image.png
