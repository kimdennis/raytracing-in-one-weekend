#include "ray.h"
#include "vec3.h"
#include <iostream>

// Generates a random point inside a unit sphere for diffuse reflection
Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = Vec3(random_double(), random_double(), random_double()) * 2.0 - Vec3(1, 1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

// Function to check if the ray hits a sphere
double hit_sphere(const Point3& center, double radius, const Ray& r) {
    Vec3 oc = r.origin() - center;
    auto a = dot(r.direction(), r.direction());
    auto b = 2.0 * dot(oc, r.direction());
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return -1.0;  // No hit
    }
    else {
        return (-b - sqrt(discriminant)) / (2.0 * a);  // Return nearest t
    }
}

// Recursive ray_color function for diffuse scattering
Color ray_color(const Ray& r, int depth) {
    if (depth <= 0) {
        return Color(0, 0, 0);  // If we've exceeded ray bounce limit, no more light is gathered.
    }

    // Check if the ray hits the main sphere
    auto t = hit_sphere(Point3(0, 0, -1), 0.5, r);
    if (t > 0.0) {
        Vec3 hit_point = r.at(t);
        Vec3 normal = unit_vector(hit_point - Vec3(0, 0, -1));
        Vec3 target = hit_point + normal + random_in_unit_sphere();
        return 0.5 * ray_color(Ray(hit_point, target - hit_point), depth - 1);  // Recursively trace scattered ray
    }

    // Check if the ray hits the ground (large sphere)
    t = hit_sphere(Point3(0, -100.5, -1), 100, r);
    if (t > 0.0) {
        Vec3 hit_point = r.at(t);
        Vec3 normal = unit_vector(hit_point - Vec3(0, -100.5, -1));
        Vec3 target = hit_point + normal + random_in_unit_sphere();
        return 0.5 * ray_color(Ray(hit_point, target - hit_point), depth - 1);  // Diffuse ground
    }

    // Background gradient (sky)
    Vec3 unit_direction = unit_vector(r.direction());
    t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);  // Sky gradient: white to blue
}

int main() {
    // Image settings
    const int image_width = 400;
    const int image_height = 225;
    const int samples_per_pixel = 100;  // For antialiasing
    const int max_depth = 50;           // Max ray bounce depth

    // Camera setup
    Point3 origin(0, 0, 0);
    Vec3 horizontal(4.0, 0.0, 0.0);
    Vec3 vertical(0.0, 2.25, 0.0);
    Point3 lower_left_corner(-2.0, -1.125, -1.0);

    // Output PPM header
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    // Rendering loop
    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);

            // Take multiple samples for each pixel and average the color
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                Ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
                pixel_color += ray_color(r, max_depth);  // Pass max_depth to control recursion
            }

            // Average the color over the number of samples
            pixel_color /= double(samples_per_pixel);

            // Gamma correction (taking the square root of the color)
            pixel_color = Color(sqrt(pixel_color.x()), sqrt(pixel_color.y()), sqrt(pixel_color.z()));

            // Output the pixel color, scaled to [0,255]
            int ir = static_cast<int>(255.999 * pixel_color.x());
            int ig = static_cast<int>(255.999 * pixel_color.y());
            int ib = static_cast<int>(255.999 * pixel_color.z());

            std::cout << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    return 0;
}
