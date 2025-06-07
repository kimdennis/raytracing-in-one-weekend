#include <stdio.h>

__global__ void test_kernel() {
    printf("Hello from GPU!\n");
}

int main() {
    test_kernel<<<1,1>>>();
    cudaDeviceSynchronize();
    return 0;
} 