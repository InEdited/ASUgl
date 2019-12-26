#include "kernels.h"
#include "util_window.h"
#include <debugapi.h>
#include <ctime>
#include <climits>

cl_uint numPlatforms;
cl_uint numDevices;

cl_platform_id platform_id;
cl_device_id device_id;
cl_context context;

int triangles_drawn = 0;

// Matrix Multiplication Variables
#define MATRIX_SIZE 16

// vertex shader
cl_mem mat_z;
cl_mem vertices_mem;
cl_mem new_vertices_mem;
bool vertex_shader_buffers_initialized = false;
//==================
// fragment shader
//	int* faces,
//	int nfaces,
//	float* new_verts,
//	int nverts,
//	int screen_width,
//	int screen_height,
//	float* z_buffer,
//	float* uniform_m,
//	float* uniform_mit,
//	float* light_dir,
//	const char* diffuse_map,
//	const char* normal_map,
//	const char* spec_map,
//	char* pixel_data
cl_mem faces_buffer;
cl_mem uniform_m;
cl_mem uniform_mit;
cl_mem light_dir_buffer;
cl_mem diffuse_map_buffer;
cl_mem norms_mem;
cl_mem spec_map_buffer;
int* hidden_pixel_buffer;
bool fragment_shader_buffers_initialized = false;
cl_mem triangles_verts_mem;
cl_mem bounding_box_min_mem;
cl_mem z_buffer_mem;
cl_mem pixel_data_buffer;
cl_mem screen_width_mem;
cl_mem nfaces_mem;
cl_mem uv_buffer;
cl_mem map_size_buffer;
//==================

cl_platform_id* platforms = NULL;
cl_device_id* devices = NULL;

cl_program vertex_shader_prog;
cl_program fragment_shader_prog;
cl_kernel vertex_shader_kernel;
cl_kernel fragment_shader_kernel;
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
	fragment_shader_prog = clCreateProgramWithSource(context, 1, (const char**)&fragment_shader_kernel_source, NULL, &err);
	err = clBuildProgram(vertex_shader_prog, 1, devices, NULL, NULL, NULL);
	err = clBuildProgram(fragment_shader_prog, 1, devices, NULL, NULL, NULL);

	if (err != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];
		printf("Error: Failed to build the fragment shader prog!\n");
		clGetProgramBuildInfo(fragment_shader_prog, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf(buffer);
		//return;
	}

	vertex_shader_kernel = clCreateKernel(vertex_shader_prog, "vertex_shader", &err);
	fragment_shader_kernel = clCreateKernel(fragment_shader_prog, "fragment_shader", &err);
	err = 0;
}

void vertex_shader(float* z, float* vertices, int vertex_count, float* new_vertices) {
	if(!vertex_shader_buffers_initialized)
	{
		mat_z            = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * MATRIX_SIZE     , NULL, &err);
		vertices_mem     = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * vertex_count * 3, NULL, &err);
		new_vertices_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * vertex_count, NULL, &err);

		err = clSetKernelArg(vertex_shader_kernel, 0, sizeof(cl_mem), &mat_z);
		err = clSetKernelArg(vertex_shader_kernel, 1, sizeof(cl_mem), &vertices_mem);
		err = clSetKernelArg(vertex_shader_kernel, 2, sizeof(cl_mem), &new_vertices_mem);
		vertex_shader_buffers_initialized = true;
	}

	err = clEnqueueWriteBuffer(commands, mat_z       , CL_TRUE, 0, sizeof(float) * MATRIX_SIZE, z, 0, NULL, NULL);
	err = clEnqueueWriteBuffer(commands, vertices_mem, CL_TRUE, 0, sizeof(float) * vertex_count * 3, vertices, 0, NULL, NULL);

	size_t vertex_shader_global[] = { vertex_count*4 };
	size_t vertex_shader_local[] = { 4 };
	err = clEnqueueNDRangeKernel(commands, vertex_shader_kernel, 1, NULL, vertex_shader_global, vertex_shader_local, 0, NULL, NULL);
}

void clear_pixel_buffer() {
	for (int i = 0; i < screen_width * screen_height; i++) {
		hidden_pixel_buffer[i] = 0;
	}
}

void clear(cl_mem* buffer, size_t size, const int pattern) {
	clEnqueueFillBuffer(commands, *buffer, &pattern, sizeof(int), 0, size, 0, NULL, NULL);
}

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
) {
	if (!fragment_shader_buffers_initialized) {

		faces_buffer       = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(cl_int3) * 3 * nfaces                , NULL, &err);
		pixel_data_buffer  = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * screen_height * screen_width  , NULL, &err);
		screen_width_mem   = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(int)                                 , NULL, &err);
		z_buffer_mem       = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * screen_height * screen_width, NULL, &err);
		nfaces_mem         = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(int)                                 , NULL, &err);
		uv_buffer          = clCreateBuffer(context, CL_MEM_READ_ONLY , uv_size                                     , NULL, &err);
		map_size_buffer    = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(int) * 2                             , NULL, &err);
		norms_mem          = clCreateBuffer(context, CL_MEM_READ_ONLY , norms_size                                  , NULL, &err);
		light_dir_buffer   = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * 3                           , NULL, &err);
		diffuse_map_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(char) * 3 * map_size[0] * map_size[1]    , NULL, &err);


		err = clSetKernelArg(fragment_shader_kernel, 0, sizeof(cl_mem), &faces_buffer);
		err = clSetKernelArg(fragment_shader_kernel, 1, sizeof(cl_mem), &new_vertices_mem);
		err = clSetKernelArg(fragment_shader_kernel, 2, sizeof(cl_mem), &pixel_data_buffer);
		err = clSetKernelArg(fragment_shader_kernel, 3, sizeof(cl_mem), &screen_width_mem);
		err = clSetKernelArg(fragment_shader_kernel, 4, sizeof(cl_mem), &z_buffer_mem);
		err = clSetKernelArg(fragment_shader_kernel, 5, sizeof(cl_mem), &nfaces_mem);
		err = clSetKernelArg(fragment_shader_kernel, 6, sizeof(cl_mem), &uv_buffer);
		err = clSetKernelArg(fragment_shader_kernel, 7, sizeof(cl_mem), &map_size_buffer);
		err = clSetKernelArg(fragment_shader_kernel, 8, sizeof(cl_mem), &light_dir_buffer);
		err = clSetKernelArg(fragment_shader_kernel, 9, sizeof(cl_mem), &norms_mem);
		err = clSetKernelArg(fragment_shader_kernel, 10, sizeof(cl_mem), &diffuse_map_buffer);

		err = clEnqueueWriteBuffer(commands, screen_width_mem  , CL_FALSE, 0, sizeof(int)                             , &screen_width , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, faces_buffer      , CL_FALSE, 0, sizeof(cl_int3) * 3 * nfaces            , faces         , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, nfaces_mem        , CL_FALSE, 0, sizeof(int)                             , &nfaces       , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, uv_buffer         , CL_FALSE, 0, uv_size                                 , uv            , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, map_size_buffer   , CL_FALSE, 0, sizeof(int) * 2                         , map_size      , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, light_dir_buffer  , CL_FALSE, 0, sizeof(float) * 3                       , light_dir     , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, norms_mem         , CL_FALSE, 0, norms_size                              , norms         , 0, NULL, NULL);
		err = clEnqueueWriteBuffer(commands, diffuse_map_buffer, CL_FALSE, 0, sizeof(char) * 3 * map_size[0] * map_size[1], diffuse_map   , 0, NULL, NULL);

		hidden_pixel_buffer = new int[screen_width*screen_height];
		fragment_shader_buffers_initialized = true;
	}

	clear(&pixel_data_buffer, sizeof(int) * screen_width * screen_height, 0);
	clear(&z_buffer_mem, sizeof(float) * screen_width * screen_height, 0);


	size_t fragment_shader_global[] = { nfaces * 256 };
	size_t framgent_shader_local[] = { 256 };

	err = clEnqueueNDRangeKernel(commands, fragment_shader_kernel, 1, NULL, fragment_shader_global, NULL, 0, NULL, NULL);

	err = clEnqueueReadBuffer(commands, pixel_data_buffer, CL_TRUE, 0, sizeof(int) * screen_height * screen_width, hidden_pixel_buffer, 0, NULL, NULL);

	for (int j = 0; j < screen_height; j++)
		for (int i = 0; i < screen_width; i++)
			set_pixel(i, j, hidden_pixel_buffer[j * screen_width + i]);
		
	

}

void destroy_kernels()
{
	clReleaseProgram(vertex_shader_prog);
	clReleaseKernel(vertex_shader_kernel);
	if (vertex_shader_buffers_initialized) {
		clReleaseMemObject(mat_z);
		clReleaseMemObject(vertices_mem);
		clReleaseMemObject(new_vertices_mem);
	}

	if (fragment_shader_buffers_initialized) {
		clReleaseMemObject(faces_buffer);
		clReleaseMemObject(screen_width_mem);
		clReleaseMemObject(z_buffer_mem);
		clReleaseMemObject(pixel_data_buffer);
		clReleaseMemObject(screen_width_mem);
		clReleaseMemObject(nfaces_mem);
		clReleaseMemObject(uv_buffer);
		clReleaseMemObject(map_size_buffer);
		clReleaseMemObject(light_dir_buffer);
		clReleaseMemObject(norms_mem);
		clReleaseMemObject(diffuse_map_buffer);
	}
	clReleaseCommandQueue(commands);

	free(hidden_pixel_buffer);


	clReleaseContext(context);
}
