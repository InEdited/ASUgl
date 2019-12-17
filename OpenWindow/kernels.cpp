#include "kernels.h"
#include <debugapi.h>

cl_uint numPlatforms;
cl_uint numDevices;

cl_platform_id platform_id;
cl_device_id device_id;
cl_context context;



// Matrix Multiplication Variables
#define ORDER 4
#define MATRIX_SIZE 16
int Mdim, Pdim, Ndim;
cl_mem mat_a;
cl_mem mat_b;
cl_mem mat_c;

// vertex shader
cl_mem mat_z;
cl_mem vertices_mem;
cl_mem new_vertices_mem;
//==================

cl_platform_id* platforms = NULL;
cl_device_id* devices = NULL;

cl_program mat_mul_prog;
cl_program vertex_shader_prog;
cl_kernel mat_mul_kernel;
cl_kernel vertex_shader_kernel;
cl_command_queue commands;

size_t mat_mul_global[DIM];
size_t mat_mul_local[DIM];

int err;


void init_kernels() {

	err = clGetPlatformIDs(0, NULL, &numPlatforms);
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numPlatforms);
	err = clGetPlatformIDs(numPlatforms, platforms, NULL);

	err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	devices = (cl_device_id*)malloc(sizeof(cl_device_id)*numDevices);
	err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);

	context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &err);

	commands = clCreateCommandQueue(context, devices[0], 0, &err);

	mat_a = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * MATRIX_SIZE, NULL, &err);
	mat_b = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * MATRIX_SIZE, NULL, &err);
	mat_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * MATRIX_SIZE, NULL, &err);


	mat_mul_prog = clCreateProgramWithSource(context, 1, (const char **)&matrix_mul_kernel_source, NULL, &err);
	vertex_shader_prog = clCreateProgramWithSource(context, 1, (const char **)&vertex_shader_kernel_source, NULL, &err);
	err = clBuildProgram(mat_mul_prog, 1, devices, NULL, NULL, NULL);
	err = clBuildProgram(vertex_shader_prog, 1, devices, NULL, NULL, NULL);

	if (err != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];
		OutputDebugString("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(mat_mul_prog, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		OutputDebugString(buffer);
		return;
	}

	mat_mul_kernel = clCreateKernel(mat_mul_prog, "mmul", &err);
	vertex_shader_kernel = clCreateKernel(vertex_shader_prog, "vertex_shader", &err);
	err = 0;
	err |= clSetKernelArg(mat_mul_kernel, 0, sizeof(cl_mem), &mat_a);
	err |= clSetKernelArg(mat_mul_kernel, 1, sizeof(cl_mem), &mat_b);
	err |= clSetKernelArg(mat_mul_kernel, 2, sizeof(cl_mem), &mat_c);
	
}

void mat4_mul(float* A, float* B, float* C)
{
	err = clEnqueueWriteBuffer(commands, mat_a, CL_TRUE, 0, sizeof(float) * MATRIX_SIZE, A, 0, NULL, NULL);
	err = clEnqueueWriteBuffer(commands, mat_b, CL_TRUE, 0, sizeof(float) * MATRIX_SIZE, B, 0, NULL, NULL);

	mat_mul_global[0] = ORDER;
	mat_mul_global[1] = ORDER;

	err = clEnqueueNDRangeKernel(commands, mat_mul_kernel, 2, NULL, mat_mul_global, NULL, 0, NULL, NULL);
	
	clFlush(commands);
	clFinish(commands);

	err = clEnqueueReadBuffer(commands, mat_c, CL_TRUE, 0, sizeof(float) * MATRIX_SIZE, C, 0, NULL, NULL);
}

void vertex_shader(Matrix* ViewPort, Matrix* Projection, Matrix* ModelView, Matrix* ModelTransform, float* vertices, int vertex_count, float* new_vertices) {

	mat_z = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * MATRIX_SIZE, NULL, &err);
	vertices_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * vertex_count * 3, NULL, &err);
	new_vertices_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * vertex_count * 3, NULL, &err);

	err = clSetKernelArg(vertex_shader_kernel, 0, sizeof(cl_mem), &mat_z);
	err = clSetKernelArg(vertex_shader_kernel, 1, sizeof(cl_mem), &vertices_mem);
	err = clSetKernelArg(vertex_shader_kernel, 2, sizeof(cl_mem), &new_vertices_mem);


	Matrix intermediate_z;
	{ // Calculate the Z to be multiplied by each vertex
		mat4_mul((float*)ViewPort       , (float*)Projection    , (float*) &intermediate_z);
		mat4_mul((float*)&intermediate_z, (float*)ModelView     , (float*) &intermediate_z);
		mat4_mul((float*)&intermediate_z, (float*)ModelTransform, (float*) &intermediate_z);
	}

	err = clEnqueueWriteBuffer(commands, mat_z, CL_TRUE, 0, sizeof(float) * MATRIX_SIZE, (float*)&intermediate_z, 0, NULL, NULL);
	err = clEnqueueWriteBuffer(commands, vertices_mem, CL_TRUE, 0, sizeof(float) * vertex_count * 3, vertices, 0, NULL, NULL);

	size_t vertex_shader_global[] = { vertex_count };
	size_t vertex_shader_local[] = { 16 };

	err = clEnqueueNDRangeKernel(commands, vertex_shader_kernel, 1, NULL, vertex_shader_global, NULL, 0, NULL, NULL);

	err = clEnqueueReadBuffer(commands, new_vertices_mem, CL_TRUE, 0, sizeof(float) * vertex_count * 3, new_vertices, 0, NULL, NULL);

	clReleaseMemObject(mat_z);
	clReleaseMemObject(vertices_mem);
	clReleaseMemObject(new_vertices_mem);
}

void destroy_kernels()
{
	clReleaseProgram(mat_mul_prog);
	clReleaseProgram(vertex_shader_prog);
	clReleaseKernel(mat_mul_kernel);
	clReleaseKernel(vertex_shader_kernel);
	clReleaseMemObject(mat_a);
	clReleaseMemObject(mat_b);
	clReleaseMemObject(mat_c);
//	clReleaseMemObject(mat_z);
//	clReleaseMemObject(vertices_mem);
//	clReleaseMemObject(new_vertices_mem);
	clReleaseCommandQueue(commands);




	clReleaseContext(context);
}
