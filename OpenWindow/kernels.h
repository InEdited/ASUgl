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

extern const char* matrix_mul_kernel_source;
// End of: Matrix Multiplication Variables

extern const char* vertex_shader_kernel_source;
extern const char* fragment_shader_kernel_source;

void init_kernels();
void vertex_shader(float* z, float* vertices, int vertex_count, float* new_vertices);
void fragment_shader(
		cl_int3* faces,
		int nfaces,
		float* uv,
		size_t uv_size,
		float* uniform_m,
		float* uniform_mit,
		float* light_dir,
		unsigned char* diffuse_map,
		float* norms,
		size_t norms_size,
		unsigned char* spec_map,
		int* map_size
);

void destroy_kernels();












#endif // !_KERNEL_HEADER_
