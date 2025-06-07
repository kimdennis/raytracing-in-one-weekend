#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <curand_kernel.h>
#include "rtweekend.cuh"
#include "vec3.cuh"
#include "ray.cuh"
#include "color.cuh"

// CUDA error checking macro
#define checkCudaErrors(val) check_cuda( (val), #val, __FILE__, __LINE__ )

void check_cuda(cudaError_t result, char const *const func, const char *const file, int const line) {
    if (result) {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
            file << ":" << line << " '" << func << "' \n";
        cudaDeviceReset();
        exit(99);
    }
}

struct hit_record {
    point3 p;
    vec3 normal;
    double t;
    bool front_face;

    __device__ __host__ inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal :-outward_normal;
    }
};

enum MaterialType { LAMBERTIAN, METAL, DIELECTRIC };

struct Material {
    MaterialType type;
    color albedo;
    double fuzz_or_ir; // Fuzz for metal, index of refraction for dielectric
};

struct Sphere {
    point3 center;
    double radius;
    Material mat;
};

struct Scene {
    Sphere* spheres;
    int num_spheres;
};

struct Camera {
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
};

__device__ __noinline__ vec3 random_in_unit_disk(curandState *local_rand_state) {
    while (true) {
        auto p = vec3(random_double(local_rand_state, -1, 1), random_double(local_rand_state, -1, 1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

__device__ double reflectance(double cosine, double ref_idx) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

// Forward declaration
__device__ color ray_color(const ray& r, Scene* world, curandState* rand_state, int depth);

__device__ bool scatter(const Material& mat, const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, curandState* rand_state) {
    if (rand_state == nullptr)
        return false;

    switch (mat.type) {
        case LAMBERTIAN: {
            auto scatter_direction = rec.normal + random_unit_vector(rand_state);
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal;
            scattered = ray(rec.p, scatter_direction);
            attenuation = mat.albedo;
            return true;
        }
        case METAL: {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + mat.fuzz_or_ir * random_in_unit_sphere(rand_state));
            attenuation = mat.albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        case DIELECTRIC: {
            attenuation = color(1.0, 1.0, 1.0);
            double refraction_ratio = rec.front_face ? (1.0/mat.fuzz_or_ir) : mat.fuzz_or_ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(fabs(1.0 - cos_theta*cos_theta));

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double(rand_state))
                direction = reflect(unit_direction, rec.normal);
            else
                direction = refract(unit_direction, rec.normal, refraction_ratio);

            scattered = ray(rec.p, direction);
            return true;
        }
        default:
            return false;
    }
}

__device__ bool hit_sphere(const Sphere& s, const ray& r, double t_min, double t_max, hit_record& rec) {
    vec3 oc = r.origin() - s.center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - s.radius*s.radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - s.center) / s.radius;
    rec.set_face_normal(r, outward_normal);
    return true;
}

__device__ bool hit_world(const Scene* world, const ray& r, double t_min, double t_max, hit_record& rec, Material& mat) {
    if (world == nullptr || world->spheres == nullptr || world->num_spheres <= 0)
        return false;

    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (int i = 0; i < world->num_spheres; i++) {
        if (hit_sphere(world->spheres[i], r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
            mat = world->spheres[i].mat;
        }
    }
    return hit_anything;
}

__device__ color ray_color(const ray& r, Scene* world, curandState* rand_state, int depth) {
    if (depth <= 0 || world == nullptr || rand_state == nullptr)
        return color(0, 0, 0);

    hit_record rec;
    Material mat;

    if (hit_world(world, r, 0.001, infinity, rec, mat)) {
        ray scattered;
        color attenuation;
        if (scatter(mat, r, rec, attenuation, scattered, rand_state)) {
            // Limit recursion depth to prevent stack overflow
            if (depth > 1) {
                return attenuation * ray_color(scattered, world, rand_state, depth - 1);
            }
        }
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}


// CUDA kernel for rendering
__global__ void render_kernel(color* d_image, Camera* cam, Scene* world, curandState* d_rand_states, int image_width, int image_height, int samples_per_pixel, int max_depth) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    if (i >= image_width || j >= image_height) return;
    if (d_image == nullptr || cam == nullptr || world == nullptr || d_rand_states == nullptr) return;

    int pixel_index = j * image_width + i;
    if (pixel_index >= image_width * image_height) return;

    curandState rand_state = d_rand_states[pixel_index];

    color pixel_color(0, 0, 0);
    for (int s = 0; s < samples_per_pixel; ++s) {
        auto u = (double(i) + random_double(&rand_state)) / (image_width - 1);
        auto v = (double(j) + random_double(&rand_state)) / (image_height - 1);
        
        vec3 rd = cam->lens_radius * random_in_unit_disk(&rand_state);
        vec3 offset = cam->u * rd.x() + cam->v * rd.y();
        ray r(cam->origin + offset, cam->lower_left_corner + u*cam->horizontal + v*cam->vertical - cam->origin - offset);

        pixel_color += ray_color(r, world, &rand_state, max_depth);
    }

    d_image[pixel_index] = pixel_color;
    d_rand_states[pixel_index] = rand_state;
}

// Host function to initialize random states
__global__ void init_rand_states(curandState* d_rand_states, unsigned long seed, int image_width, int image_height) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i >= image_width || j >= image_height) return;

    int pixel_index = j * image_width + i;
    curand_init(seed + pixel_index, 0, 0, &d_rand_states[pixel_index]);
}


// CUDA kernel to create world on device
__global__ void create_world_kernel(Scene* scene, Camera* cam, int image_width, int image_height) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        curandState rand_state;
        curand_init(1984, 0, 0, &rand_state);

        Sphere* spheres = scene->spheres;
        if (spheres == nullptr) {
            return;  // Safety check
        }

        int i = 0;
        // Ground sphere
        spheres[i++] = {point3(0, -1000, 0), 1000, {LAMBERTIAN, color(0.5, 0.5, 0.5)}};

        // Small spheres
        for (int a = -11; a < 11; a++) {
            for (int b = -11; b < 11; b++) {
                if (i >= 500) break;  // Safety check for max_spheres
                
                auto choose_mat = random_double(&rand_state);
                point3 center(a + 0.9*random_double(&rand_state), 0.2, b + 0.9*random_double(&rand_state));

                if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                    if (choose_mat < 0.8) { // diffuse
                        auto albedo = color::random(&rand_state) * color::random(&rand_state);
                        spheres[i++] = {center, 0.2, {LAMBERTIAN, albedo}};
                    } else if (choose_mat < 0.95) { // metal
                        auto albedo = color::random(&rand_state, 0.5, 1);
                        auto fuzz = random_double(&rand_state, 0, 0.5);
                        spheres[i++] = {center, 0.2, {METAL, albedo, fuzz}};
                    } else { // glass
                        spheres[i++] = {center, 0.2, {DIELECTRIC, color(), 1.5}};
                    }
                }
            }
        }
        
        // Large spheres
        if (i < 500) spheres[i++] = {point3(0, 1, 0), 1.0, {DIELECTRIC, color(), 1.5}};
        if (i < 500) spheres[i++] = {point3(-4, 1, 0), 1.0, {LAMBERTIAN, color(0.4, 0.2, 0.1)}};
        if (i < 500) spheres[i++] = {point3(4, 1, 0), 1.0, {METAL, color(0.7, 0.6, 0.5), 0.0}};

        scene->num_spheres = i;

        // Camera setup
        point3 lookfrom(13,2,3);
        point3 lookat(0,0,0);
        vec3 vup(0,1,0);
        auto dist_to_focus = 10.0;
        auto aperture = 0.1;
        auto vfov = 20.0;
        auto aspect_ratio = double(image_width) / image_height;

        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta/2);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        cam->w = unit_vector(lookfrom - lookat);
        cam->u = unit_vector(cross(vup, cam->w));
        cam->v = cross(cam->w, cam->u);

        cam->origin = lookfrom;
        cam->horizontal = dist_to_focus * viewport_width * cam->u;
        cam->vertical = dist_to_focus * viewport_height * cam->v;
        cam->lower_left_corner = cam->origin - cam->horizontal/2 - cam->vertical/2 - dist_to_focus*cam->w;
        cam->lens_radius = aperture / 2;
    }
}


int main() {
    // Set a larger stack size to accommodate deep recursion
    checkCudaErrors(cudaDeviceSetLimit(cudaLimitStackSize, 16384));

    // Image dimensions - increased for better quality
    const int image_width = 800;  // Increased from 400
    const int image_height = 450; // Increased from 225
    const int samples_per_pixel = 50; // Increased from 10
    const int max_depth = 20;     // Increased from 10
    
    std::cerr << "Rendering a " << image_width << "x" << image_height 
              << " image with " << samples_per_pixel << " samples per pixel\n";
    
    // Allocate device memory for image
    color* d_image;
    checkCudaErrors(cudaMalloc(&d_image, image_width * image_height * sizeof(color)));

    // Allocate device memory for random states
    curandState* d_rand_states;
    checkCudaErrors(cudaMalloc(&d_rand_states, image_width * image_height * sizeof(curandState)));

    // Initialize random states
    dim3 block(16, 16);  // Increased block size
    dim3 grid((image_width + block.x - 1) / block.x, (image_height + block.y - 1) / block.y);
    init_rand_states<<<grid, block>>>(d_rand_states, time(0), image_width, image_height);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());
    
    std::cerr << "Creating world...\n";

    // Scene setup
    const int max_spheres = 500;
    Sphere* d_spheres;
    checkCudaErrors(cudaMalloc(&d_spheres, max_spheres * sizeof(Sphere)));
    
    Scene* d_scene;
    checkCudaErrors(cudaMalloc(&d_scene, sizeof(Scene)));

    // Create a host-side Scene struct and copy it to the device
    Scene h_scene;
    h_scene.spheres = d_spheres;
    h_scene.num_spheres = 0; // Will be updated by the kernel
    checkCudaErrors(cudaMemcpy(d_scene, &h_scene, sizeof(Scene), cudaMemcpyHostToDevice));

    Camera* d_camera;
    checkCudaErrors(cudaMalloc(&d_camera, sizeof(Camera)));

    // Create world and camera
    create_world_kernel<<<1,1>>>(d_scene, d_camera, image_width, image_height);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    // Verify the scene was created properly
    Scene h_verify_scene;
    checkCudaErrors(cudaMemcpy(&h_verify_scene, d_scene, sizeof(Scene), cudaMemcpyDeviceToHost));
    if (h_verify_scene.num_spheres == 0) {
        std::cerr << "Error: No spheres were created in the scene!\n";
        return 1;
    }
    std::cerr << "Created scene with " << h_verify_scene.num_spheres << " spheres\n";

    std::cerr << "Rendering...\n";
    
    // Record start time
    cudaEvent_t start, stop;
    checkCudaErrors(cudaEventCreate(&start));
    checkCudaErrors(cudaEventCreate(&stop));
    checkCudaErrors(cudaEventRecord(start, 0));

    // Render with optimized block size
    dim3 render_block(16, 16);  // Increased block size
    dim3 render_grid((image_width + render_block.x - 1) / render_block.x, 
                    (image_height + render_block.y - 1) / render_block.y);
    
    std::cerr << "Launching kernel with grid size: " << render_grid.x << "x" << render_grid.y 
              << " and block size: " << render_block.x << "x" << render_block.y << "\n";
    
    render_kernel<<<render_grid, render_block>>>(d_image, d_camera, d_scene, d_rand_states, 
                                               image_width, image_height, samples_per_pixel, max_depth);
    
    // Check for kernel launch errors
    cudaError_t kernelError = cudaGetLastError();
    if (kernelError != cudaSuccess) {
        std::cerr << "Kernel launch error: " << cudaGetErrorString(kernelError) << "\n";
        return 1;
    }
    
    // Synchronize and check for errors
    cudaError_t syncError = cudaDeviceSynchronize();
    if (syncError != cudaSuccess) {
        std::cerr << "Device synchronization error: " << cudaGetErrorString(syncError) << "\n";
        return 1;
    }

    // Record end time
    checkCudaErrors(cudaEventRecord(stop, 0));
    checkCudaErrors(cudaEventSynchronize(stop));
    float milliseconds = 0;
    checkCudaErrors(cudaEventElapsedTime(&milliseconds, start, stop));
    std::cerr << "\nDone. Took " << milliseconds/1000.0f << " seconds.\n";

    // Copy result back to host
    color* h_image = new color[image_width * image_height];
    checkCudaErrors(cudaMemcpy(h_image, d_image, image_width * image_height * sizeof(color), cudaMemcpyDeviceToHost));

    // Write to PPM file
    const char* filepath = "cuda_image.ppm";
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing!\n";
    } else {
        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = image_height - 1; j >= 0; --j) {
            for (int i = 0; i < image_width; ++i) {
                write_color(out, h_image[j * image_width + i], samples_per_pixel);
            }
        }
        out.close();
        std::cerr << "Image saved to: " << filepath << "\n";
    }

    // Cleanup
    delete[] h_image;
    checkCudaErrors(cudaFree(d_camera));
    checkCudaErrors(cudaFree(d_spheres));
    checkCudaErrors(cudaFree(d_scene));
    checkCudaErrors(cudaFree(d_image));
    checkCudaErrors(cudaFree(d_rand_states));

    return 0;
} 