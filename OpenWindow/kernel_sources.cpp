#include "kernels.h"

const char* vertex_shader_kernel_source =
"__kernel                                                                                                                    \n"
"void vertex_shader( __global float* m,                                                                                      \n"
"                    __global float* VertexBuffer,                                                                           \n"
"                    __global float* NewVertexBuffer)                                                                        \n"
"{                                                                                                                           \n"
"    int local_index = get_local_id(0);                                                                                      \n"
"    int global_index = get_group_id(0);                                                                                     \n"
"    NewVertexBuffer[4*global_index+local_index] =                                                                           \n"
"						  m[local_index*4]*VertexBuffer[3*global_index]                                                      \n"
"	                    + m[local_index*4 + 1]*VertexBuffer[3*global_index+1]                                                \n"
"	                    + m[local_index*4 + 2]*VertexBuffer[3*global_index+2]                                                \n"
"	                    + m[local_index*4 + 3];                                                                              \n"
"}                                                                                                                           \n";
