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

int* hidden_pixel_buffer;
cl_mem z_buffer_mem;
cl_mem pixel_data_buffer;
//==================

cl_platform_id* platforms = NULL;
cl_device_id* devices = NULL;

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

	hidden_pixel_buffer = new int[screen_width*screen_height];
	pixel_data_buffer  = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * screen_height * screen_width  , NULL, &err);
	z_buffer_mem       = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * screen_height * screen_width, NULL, &err);
}


void clear(cl_mem* buffer, size_t size, const int pattern) {
	clEnqueueFillBuffer(commands, *buffer, &pattern, sizeof(int), 0, size, 0, NULL, NULL);
}

void new_frame() {
	clear(&pixel_data_buffer, sizeof(int) * screen_width * screen_height, 0);
	clear(&z_buffer_mem, sizeof(float) * screen_width * screen_height, 0);
}

void end_frame() {
	clFinish(commands);
	clEnqueueReadBuffer(commands, pixel_data_buffer, CL_TRUE, 0, sizeof(int) * screen_height * screen_width, hidden_pixel_buffer, 0, NULL, NULL);

	for (int j = 0; j < screen_height; j++)
		for (int i = 0; i < screen_width; i++)
			set_pixel(i, j, hidden_pixel_buffer[j * screen_width + i]);
}

void destroy_kernels()
{
	clReleaseMemObject(z_buffer_mem);
	clReleaseMemObject(pixel_data_buffer);
	clReleaseCommandQueue(commands);

	free(hidden_pixel_buffer);

	clReleaseContext(context);
}
