#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <curand_kernel.h>


// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants
#ifdef __CUDA_ARCH__
    __device__ const double pi = 3.1415926535897932385;
    __device__ const double infinity = HUGE_VAL;
#else
    const double pi = 3.1415926535897932385;
    const double infinity = std::numeric_limits<double>::infinity();
#endif

// Utility Functions
__device__ __host__ inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// Forward declarations
class ray;
struct hit_record;

__device__ inline double random_double(curandState *local_rand_state);

__device__ inline double random_double(curandState *local_rand_state) {
    return curand_uniform_double(local_rand_state);
}

__device__ inline double random_double(curandState *local_rand_state, double min, double max) {
    return min + (max-min)*random_double(local_rand_state);
}

// Common Headers
#include "ray.cuh"
// vec3.cuh is included by ray.cuh

#endif 