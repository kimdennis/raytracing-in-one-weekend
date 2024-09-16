#include <iostream>
#include "vec3.h"
#include "ray.h"

// ray_color function calculates color based on ray direction
Color ray_color(const Ray& r) {
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main() {
    // Image dimensions
    const int image_width = 400;
    const int image_height = 200;

    // Camera setup
    Point3 origin(0, 0, 0);
    Vec3 horizontal(4.0, 0.0, 0.0);
    Vec3 vertical(0.0, 2.0, 0.0);
    Point3 lower_left_corner(-2.0, -1.0, -1.0);

    // Output the PPM header
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    // Rendering loop
    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            auto u = double(i) / (image_width - 1);
            auto v = double(j) / (image_height - 1);
            Ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
            Color pixel_color = ray_color(r);
            std::cout << static_cast<int>(255.999 * pixel_color.x()) << ' '
                << static_cast<int>(255.999 * pixel_color.y()) << ' '
                << static_cast<int>(255.999 * pixel_color.z()) << '\n';
        }
    }

    return 0;
}
