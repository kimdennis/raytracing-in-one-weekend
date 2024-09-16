#include <iostream>
#include "vec3.h"
#include "ray.h"

double hit_sphere(const Point3& center, double radius, const Ray& r) {
    Vec3 oc = r.origin() - center;
    auto a = dot(r.direction(), r.direction());
    auto b = 2.0 * dot(oc, r.direction());
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return -1.0;  // No intersection
    }
    else {
        return (-b - sqrt(discriminant)) / (2.0 * a);  // Return nearest t
    }
}

Color ray_color(const Ray& r) {
    double t_closest = std::numeric_limits<double>::max();  // Keep track of the closest hit
    Point3 sphere_center_1(0, 0, -1);  // First (and only) sphere

    // Check if the ray hits the first sphere
    double t1 = hit_sphere(sphere_center_1, 0.5, r);
    if (t1 > 0.0 && t1 < t_closest) {
        t_closest = t1;
        Vec3 N = unit_vector(r.at(t1) - sphere_center_1);
        return 0.5 * Color(N.x() + 1, N.y() + 1, N.z() + 1);  // Shade the first sphere
    }

    // Background gradient if no hit
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
