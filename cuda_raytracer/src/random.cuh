#pragma once

#include <curand_kernel.h>
#include "vec3.cuh"
#include "rtweekend.cuh"

class RandomState {
public:
    __device__ RandomState(curandState* state) : state(state) {}

    __device__ float random_float() {
        return random_double(state);
    }

    __device__ float random_float(float min, float max) {
        return random_double(state, min, max);
    }

    __device__ vec3 random_unit_vector() {
        return ::random_unit_vector(state);
    }

    __device__ vec3 random_in_unit_sphere() {
        return ::random_in_unit_sphere(state);
    }

    __device__ vec3 random_in_unit_disk() {
        return ::random_in_unit_disk(state);
    }

    __device__ vec3 random_vec3() {
        return vec3(random_float(), random_float(), random_float());
    }

    __device__ vec3 random_vec3(float min, float max) {
        return vec3(random_float(min, max), random_float(min, max), random_float(min, max));
    }

    __device__ vec3 random_in_hemisphere(const vec3& normal) {
        vec3 in_unit_sphere = random_in_unit_sphere();
        if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
            return in_unit_sphere;
        else
            return -in_unit_sphere;
    }

private:
    curandState* state;
}; 