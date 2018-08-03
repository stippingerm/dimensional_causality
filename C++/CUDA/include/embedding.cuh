#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#ifndef EMBEDDING_H
#define EMBEDDING_H

__global__ void single_embedding_kernel(float* state_space, float* x, int emb_dim, int a, unsigned int tau);
__global__ void joint_embedding_kernel(float* state_space, float* x, float* y, int emb_dim, int a, unsigned int tau);
__global__ void shuffled_embedding_kernel(float* state_space, float* state_space_X, float* state_space_Y, int* shuffled_indices, int emb_dim);
__global__ void init_indices(int* indices);
cudaError_t dev_alloc_timeseries(float** dev_x, float** dev_y, int dev_x_size);
cudaError_t dev_copy_timeseries(float* dev_x, float* dev_y, float* host_x, float* host_y, int n);
cudaError_t dev_alloc_manifolds(float** dev_state_space_X, float** dev_state_space_Y, float** dev_state_space_J, float** dev_state_space_Z, int dev_state_space_size);
cudaError_t random_shuffle(int** indices, int num_blocks, int num_threads_per_block, int dev_state_space_size);
cudaError_t embed_manifolds(float** dev_state_space_X, float** dev_state_space_Y, float** dev_state_space_J, float** dev_state_space_Z, int& dev_state_space_size, float* host_x, float* host_y, int n, int& dev_x_size, int emb_dim, int tau, int num_threads_per_block);

#endif