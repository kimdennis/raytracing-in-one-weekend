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
#include <chrono>

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
    const int image_width = 800;
    const int image_height = 450;
    const int samples_per_pixel = 50;
    const int max_depth = 10;

    // World setup
    HittableList world;

    // Ground Material
    auto ground_material = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    // Random Spheres
    for (int a = -5; a < 5; a++) {
        for (int b = -5; b < 5; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8) {
                    // Diffuse
                    auto albedo = Color::random() * Color::random();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // Metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<Metal>(albedo, fuzz);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
                else {
                    // Glass
                    sphere_material = std::make_shared<Dielectric>(1.5);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    // Three Main Spheres
    auto material1 = std::make_shared<Dielectric>(1.5);
    world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    // Camera configuration
    Point3 lookfrom(13, 2, 3); // Camera position
    Point3 lookat(0, 0, 0);    // Look-at point
    Vec3 vup(0, 1, 0);         // Up vector
    auto dist_to_focus = 10.0; // Focus distance
    auto aperture = 0.1;        // Aperture size for depth of field
    auto vfov = 20.0;           // Vertical field of view in degrees

    Camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
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

    // End timing and calculate duration
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cerr << "\nDone.\n";
    std::cerr << "Rendering completed in " << duration.count() << " milliseconds (" 
              << (duration.count() / 1000.0) << " seconds)\n";
    std::cerr << "Image: " << image_width << "x" << image_height 
              << " pixels, " << samples_per_pixel << " samples per pixel\n";
    std::cerr << "Total rays traced: " << (image_width * image_height * samples_per_pixel) << "\n";
    
    return 0;
}
