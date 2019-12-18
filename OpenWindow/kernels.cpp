#include "kernels.h"
#include <debugapi.h>

cl_uint numPlatforms;
cl_uint numDevices;

cl_platform_id platform_id;
cl_device_id device_id;
cl_context context;



// Matrix Multiplication Variables
#define MATRIX_SIZE 16

// vertex shader
cl_mem mat_z;
cl_mem vertices_mem;
cl_mem new_vertices_mem;
//==================

cl_platform_id* platforms = NULL;
cl_device_id* devices = NULL;

cl_program vertex_shader_prog;
cl_kernel vertex_shader_kernel;
cl_command_queue commands;

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

	vertex_shader_prog = clCreateProgramWithSource(context, 1, (const char **)&vertex_shader_kernel_source, NULL, &err);
	err = clBuildProgram(vertex_shader_prog, 1, devices, NULL, NULL, NULL);

	if (err != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];
		printf("Error: Failed to build program executable!\n");
		//clGetProgramBuildInfo(mat_mul_prog, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf(buffer);
		return;
	}

	vertex_shader_kernel = clCreateKernel(vertex_shader_prog, "vertex_shader", &err);
	err = 0;
}

void vertex_shader(float* z, float* vertices, int vertex_count, float* new_vertices) {
	mat_z            = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * MATRIX_SIZE     , NULL, &err);
	vertices_mem     = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * vertex_count * 3, NULL, &err);
	new_vertices_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * vertex_count * 4, NULL, &err);

	err = clSetKernelArg(vertex_shader_kernel, 0, sizeof(cl_mem), &mat_z);
	err = clSetKernelArg(vertex_shader_kernel, 1, sizeof(cl_mem), &vertices_mem);
	err = clSetKernelArg(vertex_shader_kernel, 2, sizeof(cl_mem), &new_vertices_mem);

	err = clEnqueueWriteBuffer(commands, mat_z       , CL_TRUE, 0, sizeof(float) * MATRIX_SIZE, z, 0, NULL, NULL);
	err = clEnqueueWriteBuffer(commands, vertices_mem, CL_TRUE, 0, sizeof(float) * vertex_count * 3, vertices, 0, NULL, NULL);

	size_t vertex_shader_global[] = { vertex_count*4 };
	size_t vertex_shader_local[] = { 4 };

	err = clEnqueueNDRangeKernel(commands, vertex_shader_kernel, 1, NULL, vertex_shader_global, vertex_shader_local, 0, NULL, NULL);

	clFlush(commands);
	clFinish(commands);
	err = clEnqueueReadBuffer(commands, new_vertices_mem, CL_TRUE, 0, sizeof(float) * vertex_count * 4, new_vertices, 0, NULL, NULL);

	clReleaseMemObject(mat_z);
	clReleaseMemObject(vertices_mem);
	clReleaseMemObject(new_vertices_mem);
}

void destroy_kernels()
{
	clReleaseProgram(vertex_shader_prog);
	clReleaseKernel(vertex_shader_kernel);
//	clReleaseMemObject(mat_z);
//	clReleaseMemObject(vertices_mem);
//	clReleaseMemObject(new_vertices_mem);
	clReleaseCommandQueue(commands);




	clReleaseContext(context);
}
