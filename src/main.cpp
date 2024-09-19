// main.cpp

#include "rtweekend.h"
#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <iostream>

// Function to compute the color seen by a ray
Color ray_color(const Ray& r, const Hittable& world, int depth) {
    HitRecord rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return Color(0, 0, 0);

    // If the ray hits something in the world
    if (world.hit(r, 0.001, infinity, rec)) {
        Ray scattered;
        Color attenuation;

        // If the material scatters the ray, recursively compute the color
        if (rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);

        // If the ray is absorbed, return black
        return Color(0, 0, 0);
    }

    // Background gradient (sky)
    Vec3 unit_direction = unit_vector(r.direction());
    double t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main() {
    // Image configuration
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 500; // Higher sample count for better image quality
    const int max_depth = 50; // Maximum recursion depth for ray bounces

    // World setup
    HittableList world;

    // Materials
    auto material_ground = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.0)); // Ground sphere
    auto material_center = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5)); // Center sphere
    auto material_left = std::make_shared<Dielectric>(1.5); // Left sphere (glass)
    auto material_right = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0); // Right sphere (metal)

    // Objects
    world.add(std::make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground)); // Ground
    world.add(std::make_shared<Sphere>(Point3(0.0, 0.0, -1.0), 0.5, material_center)); // Center

    // Hollow Glass Sphere (Left)
    world.add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.6, material_left));  // Outer sphere
    world.add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), -0.5, material_left)); // Inner sphere (hollow)

    world.add(std::make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right)); // Right sphere

    // Camera configuration
    Point3 lookfrom(3, 3, 2); // Camera position
    Point3 lookat(0, 0, -1); // Look-at point
    Vec3 vup(0, 1, 0); // Up vector
    auto dist_to_focus = (lookfrom - lookat).length(); // Focus distance
    auto aperture = 0.1; // Aperture size for depth of field
    auto vfov = 40.0; // Vertical field of view in degrees

    Camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j) {
        // Progress indicator
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);

            // Accumulate color over multiple samples per pixel
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                Ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }

            // Write the color to the output
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
    return 0;
}
