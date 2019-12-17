#ifndef _KERNEL_HEADER_
#define _KERNEL_HEADER_
#include <Windows.h>
#include <CL/cl.h>
#include "geometry.h"


extern cl_uint numPlatforms;
extern cl_platform_id firstPlatformId;
extern cl_device_id device_id;
extern cl_context context;



// Matrix Multiplication Variables
#define ORDER 4
#define MATRIX_SIZE 16
extern int Mdim, Pdim, Ndim;
extern cl_mem mat_a;
extern cl_mem mat_b;
extern cl_mem mat_c;

extern cl_program mat_mul_prog;
extern cl_kernel mat_mul_kernel;
extern cl_command_queue commands;

extern int err;


#define DIM 2

extern size_t mat_mul_global[DIM];
extern size_t mat_mul_local[DIM];

extern const char* matrix_mul_kernel_source;
// End of: Matrix Multiplication Variables

extern const char* vertex_shader_kernel_source;

void init_kernels();
void mat4_mul(float* A, float* B, float* C);
void vertex_shader(Matrix* ViewPort, Matrix* Projection, Matrix* ModelView, Matrix* ModelTransform, float* vertices, int vertex_count, float* new_vertices);
void destroy_kernels();












#endif // !_KERNEL_HEADER_
