#include <iostream>
#include <fstream>
#include <sstream>
#include "model.h"

#define PI 3.14159265358979323846
#define DEG2RAD PI/180

Model::Model(const char *filename, int watery) : verts_(), faces_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_() {
	fluid = watery;
    std::ifstream in;
	Transform = Matrix::identity();
	Rotation = Matrix::identity();
	Scale = Matrix::identity();
	Translation = Matrix::identity();
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i=0;i<3;i++) iss >> n[i];
            norms_.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i=0;i<2;i++) iss >> uv[i];
            uv_.push_back(uv);
        }  else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i=0; i<3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    load_texture(filename, "_diffuse.tga", diffusemap_);
	init_kernels();
}

Model::~Model() {
	release_kernels();
}

void Model::ApplyTransform() {
	Transform = Translation * Scale * Rotation;
}

void Model::translate(Vec3f tr) {
	Translation[0][3] = tr.x;
	Translation[1][3] = tr.y;
	Translation[2][3] = tr.z;
}
void Model::rotate(Vec3f rot) {
	rot = rot * DEG2RAD;

	Rotation[0][0] = cosf(rot.y) * cosf(rot.z);
	Rotation[0][1] = -cosf(rot.y) * sinf(rot.z);
	Rotation[0][2] = sinf(rot.y);
	Rotation[1][0] = sinf(rot.x)*sinf(rot.y)*cosf(rot.z) + cosf(rot.x) * sinf(rot.z);
	Rotation[1][1] = -sinf(rot.x)*sinf(rot.y)*sinf(rot.z) + cosf(rot.x) * cosf(rot.z);
	Rotation[1][2] = -sinf(rot.x)*cosf(rot.y);
	Rotation[2][0] = -cosf(rot.x)*sinf(rot.y)*cosf(rot.z) + sinf(rot.x) * sinf(rot.z);
	Rotation[2][1] = cosf(rot.x)*sinf(rot.y)*sinf(rot.z) + sinf(rot.x) * cosf(rot.z);
	Rotation[2][2] = cosf(rot.x)*cosf(rot.y);
}
void Model::scale(Vec3f scl) {
	Scale[0][0] = scl.x;
	Scale[1][1] = scl.y;
	Scale[2][2] = scl.z;
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    std::vector<int> face;
    for (int i=0; i<(int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
    return face;
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert][0]];
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        //img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2f uvf) {
    Vec2i uv(uvf[0]*diffusemap_.get_width(), uvf[1]*diffusemap_.get_height());
    return diffusemap_.get(uv[0], uv[1]);
}

Vec3f Model::normal(Vec2f uvf) {
    Vec2i uv(uvf[0]*normalmap_.get_width(), uvf[1]*normalmap_.get_height());
    TGAColor c = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (int i=0; i<3; i++)
        res[2-i] = (float)c[i]/255.f*2.f - 1.f;
    return res;
}

Vec2f Model::uv(int iface, int nthvert) {
    return uv_[faces_[iface][nthvert][1]];
}

float Model::specular(Vec2f uvf) {
    Vec2i uv(uvf[0]*specularmap_.get_width(), uvf[1]*specularmap_.get_height());
    return specularmap_.get(uv[0], uv[1])[0]/1.f;
}

Vec3f Model::normal(int iface, int nthvert) {
    int idx = faces_[iface][nthvert][2];
    return norms_[idx].normalize();
}

void Model::init_kernels() {
	if(fluid)
		vertex_shader_prog = clCreateProgramWithSource(context, 1, (const char **)&fluid_vertex_shader_kernel_source, NULL, &err);
	else
		vertex_shader_prog = clCreateProgramWithSource(context, 1, (const char **)&vertex_shader_kernel_source, NULL, &err);
	err = clBuildProgram(vertex_shader_prog, 1, devices, NULL, NULL, NULL);
	vertex_shader_kernel = clCreateKernel(vertex_shader_prog, "vertex_shader", &err);
	vertex_shader_matz = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * 16, NULL, &err);
	vertex_shader_vertices = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * nverts() * 3, NULL, &err);
	new_vertices_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * nverts(), NULL, &err);

	clSetKernelArg(vertex_shader_kernel, 0, sizeof(cl_mem), &vertex_shader_matz);
	clSetKernelArg(vertex_shader_kernel, 1, sizeof(cl_mem), &vertex_shader_vertices);
	clSetKernelArg(vertex_shader_kernel, 2, sizeof(cl_mem), &new_vertices_mem);
	clSetKernelArg(vertex_shader_kernel, 3, sizeof(cl_mem), &time_buffer);
	clEnqueueWriteBuffer(commands, vertex_shader_vertices, CL_TRUE, 0, sizeof(float) * nverts() * 3, *(float**)((Vec3f*) &verts_), 0, NULL, NULL);

	int map_size[] = { diffusemap_.get_width(), diffusemap_.get_height() };
	faces = (cl_int3*)malloc(3 * sizeof(cl_int3) * nfaces());
	for (int i = 0; i < nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			faces[i * 3 + j].x = faces_[i][j][0];
			faces[i * 3 + j].y = faces_[i][j][1];
			faces[i * 3 + j].z = faces_[i][j][2];
		}
	}

	fragment_shader_prog = clCreateProgramWithSource(context, 1, (const char**)&fragment_shader_kernel_source, NULL, &err);
	err = clBuildProgram(fragment_shader_prog, 1, devices, NULL, NULL, NULL);
	fragment_shader_kernel = clCreateKernel(fragment_shader_prog, "fragment_shader", &err);

	fragment_shader_faces        = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(cl_int3) * 3 * nfaces()               , NULL, &err);
	fragment_shader_screen_width = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(int)                                  , NULL, &err);
	fragment_shader_uv           = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * uv_.size() * 2               , NULL, &err);
	fragment_shader_map_size     = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(int) * 2                              , NULL, &err);
	fragment_shader_norms        = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * norms_.size() * 3            , NULL, &err);
	fragment_shader_light_dir    = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(float) * 3                            , NULL, &err);
	fragment_shader_diffuse_map  = clCreateBuffer(context, CL_MEM_READ_ONLY , sizeof(char) * 3 * map_size[0] * map_size[1] , NULL, &err);

	clSetKernelArg(fragment_shader_kernel, 0, sizeof(cl_mem), &fragment_shader_faces);
	clSetKernelArg(fragment_shader_kernel, 1, sizeof(cl_mem), &new_vertices_mem);
	clSetKernelArg(fragment_shader_kernel, 2, sizeof(cl_mem), &pixel_data_buffer);
	clSetKernelArg(fragment_shader_kernel, 3, sizeof(cl_mem), &fragment_shader_screen_width);
	clSetKernelArg(fragment_shader_kernel, 4, sizeof(cl_mem), &z_buffer_mem);
	clSetKernelArg(fragment_shader_kernel, 5, sizeof(cl_mem), &fragment_shader_uv);
	clSetKernelArg(fragment_shader_kernel, 6, sizeof(cl_mem), &fragment_shader_map_size);
	clSetKernelArg(fragment_shader_kernel, 7, sizeof(cl_mem), &fragment_shader_light_dir);
	clSetKernelArg(fragment_shader_kernel, 8, sizeof(cl_mem), &fragment_shader_norms);
	clSetKernelArg(fragment_shader_kernel, 9, sizeof(cl_mem), &fragment_shader_diffuse_map);

	clEnqueueWriteBuffer(commands, fragment_shader_screen_width, CL_FALSE, 0, sizeof(int)                                 , &screen_width              , 0, NULL, NULL);
	clEnqueueWriteBuffer(commands, fragment_shader_faces       , CL_FALSE, 0, sizeof(cl_int3) * 3 * nfaces()              , faces                      , 0, NULL, NULL);
	clEnqueueWriteBuffer(commands, fragment_shader_uv          , CL_FALSE, 0, sizeof(float) * 2 * uv_.size()              , *(float**)((Vec2f*)&uv_)   , 0, NULL, NULL);
	clEnqueueWriteBuffer(commands, fragment_shader_map_size    , CL_FALSE, 0, sizeof(int) * 2                             , map_size                   , 0, NULL, NULL);
	clEnqueueWriteBuffer(commands, fragment_shader_norms       , CL_FALSE, 0, sizeof(float) * norms_.size() * 3           , *(float**)((Vec3f*)&norms_), 0, NULL, NULL);
	clEnqueueWriteBuffer(commands, fragment_shader_diffuse_map , CL_TRUE, 0, sizeof(char) * 3 * map_size[0] * map_size[1], diffusemap_.data           , 0, NULL, NULL);
}

void Model::vertex(float* z) {
	size_t vertex_shader_global[] = { nverts() * 4 };
	size_t vertex_shader_local[] = { 4 };

	clEnqueueWriteBuffer(commands, vertex_shader_matz, CL_TRUE, 0, sizeof(float) * 16, z, 0, NULL, NULL);

	clEnqueueNDRangeKernel(commands, vertex_shader_kernel, 1, NULL, vertex_shader_global, vertex_shader_local, 0, NULL, NULL);
}
void Model::fragment(float* light_dir) {
	clEnqueueWriteBuffer(commands, fragment_shader_light_dir, CL_FALSE, 0, sizeof(float) * 3, light_dir, 0, NULL, NULL);
	size_t fragment_shader_global[] = { nfaces() * 256 };
	size_t fragment_shader_local[] = { 256 };

	clEnqueueNDRangeKernel(commands, fragment_shader_kernel, 1, NULL, fragment_shader_global, fragment_shader_local, 0, NULL, NULL);

}
void Model::render(Matrix* z, float* light_dir) {
	Matrix m = *z * Transform;
	vertex((float*)&m);
	clFinish(commands);
	fragment(light_dir);
}
void Model::release_kernels() {

	clReleaseProgram(vertex_shader_prog);
	clReleaseKernel(vertex_shader_kernel);
	clReleaseMemObject(vertex_shader_matz);
	clReleaseMemObject(vertex_shader_vertices);
	clReleaseMemObject(new_vertices_mem);

	clReleaseProgram(fragment_shader_prog);
	clReleaseKernel(fragment_shader_kernel);
	clReleaseMemObject(fragment_shader_faces);
	clReleaseMemObject(fragment_shader_screen_width);
	clReleaseMemObject(fragment_shader_uv);
	clReleaseMemObject(fragment_shader_map_size);
	clReleaseMemObject(fragment_shader_norms);
	clReleaseMemObject(fragment_shader_light_dir);
	clReleaseMemObject(fragment_shader_diffuse_map);

	free(faces);
}


const char* fluid_vertex_shader_kernel_source =
"__kernel                                                                                                                    \n"
"void vertex_shader( __global float* m,                                                                                      \n"
"                    __global float* VertexBuffer,                                                                           \n"
"                    __global float* NewVertexBuffer,                                                                        \n"
"                    __global float* TIME           )                                                                        \n"
"{                                                                                                                           \n"
"    int local_index = get_local_id(0);                                                                                      \n"
"    int global_index = get_group_id(0);                                                                                     \n"
"    float AMPLITUDE = 5;                                                                                     \n"
"    float FREQ = 2.0f;                                                                                     \n"
"                                                                                                                            \n"
"    float offset = 0.f;                                                                                                     \n"
"    if(local_index == 1) {                                                                                                  \n"
"    offset = (sin(TIME[0]*(fmod(10-VertexBuffer[3*global_index],5.f) * FREQ)) + sin(TIME[0]*(fmod(10-VertexBuffer[3*global_index+2], 5.f))*FREQ)) * AMPLITUDE;                                                                                                \n"
"    }                                                                                                                       \n"
"                                                                                                                            \n"
"    NewVertexBuffer[4*global_index+local_index] =                                                                           \n"
"						  m[local_index*4]*VertexBuffer[3*global_index]                                                      \n"
"	                    + m[local_index*4 + 1]*VertexBuffer[3*global_index+1]                                                \n"
"	                    + m[local_index*4 + 2]*VertexBuffer[3*global_index+2]                                                \n"
"	                    + m[local_index*4 + 3]                                                                               \n"
"	                    + offset;                                                                                            \n"
"}                                                                                                                           \n";


const char* vertex_shader_kernel_source =
"__kernel                                                                                                                    \n"
"void vertex_shader( __global float* m,                                                                                      \n"
"                    __global float* VertexBuffer,                                                                           \n"
"                    __global float* NewVertexBuffer,                                                                        \n"
"                    __global float* TIME           )                                                                        \n"
"{                                                                                                                           \n"
"    int local_index = get_local_id(0);                                                                                      \n"
"    int global_index = get_group_id(0);                                                                                     \n"
"                                                                                                                            \n"
"    NewVertexBuffer[4*global_index+local_index] =                                                                           \n"
"						  m[local_index*4]*VertexBuffer[3*global_index]                                                      \n"
"	                    + m[local_index*4 + 1]*VertexBuffer[3*global_index+1]                                                \n"
"	                    + m[local_index*4 + 2]*VertexBuffer[3*global_index+2]                                                \n"
"	                    + m[local_index*4 + 3];                                                                              \n"
"}                                                                                                                           \n";

const char* fragment_shader_kernel_source =
"float3 barycentric(float3* pts, float3 P)                                                                                                                                                                    \n"
"{                                                                                                                                                                                                          \n"
"    float3 u = cross(                                                                                                                                                                                      \n"
"        (float3){pts[0][2] - pts[0][0], pts[0][1] - pts[0][0], pts[0][0] - P[0]}, // AC_x, AB_x, distance_x                                                                                                \n"
"        (float3){pts[1][2] - pts[1][0], pts[1][1] - pts[1][0], pts[1][0] - P[1]}  // AC_y, AB_y, distance_y                                                                                                \n"
"    );                                                                                                                                                                                                     \n"
"	 if (fabs(u[2]) < 1) return (float3){-1, 1, 1};                                                                                                                                                         \n"
"	 return (float3){1.f - (u[0] + u[1]) / u[2], u[1] / u[2], u[0] / u[2]};                                                                                                                                 \n"
"}                                                                                                                                                                                                          \n"
"                                                                                                                                                                                                    \n"
"__kernel void fragment_shader (                                                                                                                                                                \n"
"    __global int3* faces,                                                                                                                                                                        \n"
"    __global float* vertices,                                                                                                                                                                             \n"
"    __global int* pixels,                                                                                                                                                                  \n"
"    __global int* screen_width,                                                                                                                                                                  \n"
"    __global float* z_buffer,                                                                                                                                                                         \n"
"    __global float* uv_buffer,                                                                                                                                                                        \n"
"    __global int* map_size,                                                                                                                                                                       \n"
"    __global float* light_dir,                                                                                                                                                                           \n"
"    __global float* norms_buff,                                                                                                                                                                           \n"
"    __global uchar* diffuse_map                                                                                                                                                                              \n"
") {                                                                                                                                                                                                        \n"
"    int GROUP_ID = get_group_id(0);                                                                                                                                                                          \n"
"    int GROUP_SIZE = get_local_size(0);                                                                                                                                                                          \n"
"    int LOCAL_ID = get_local_id(0);                                                                                                                                                                          \n"
"                                                                                                                                                                                                           \n"
"    bool out = true;                                                                                                                                                                                       \n"
"    float3 vertices3[3];                                                                                                                                                                                   \n"
"    float2 uv_coords[3];                                 \n"
"    float3 norms[3];                                                                                                                                                                                   \n"
"                                                                                                                                                                                                           \n"
"    for(int i = 0; i < 3; i++) {                                                                                                                                                                           \n"
"        float4 vertex;// =  vertices[faces[GROUP_ID * 3 + i ][0]];                                                                                                                                                \n"
"        for(int j = 0; j < 4; j ++) {                                                                                                                                                                                                   \n"
"            vertex[j] = vertices[4 * faces[GROUP_ID*3 + i][0] + j];                                                                                                                                          \n"
"        }                                                                                                                                                                                                  \n"
"                                                                                                                                                                                                           \n"
"        for(int j = 0; j < 2; j++) {                                                                                                                                                                       \n"
"            uv_coords[i][j]  = uv_buffer [2 * faces[GROUP_ID * 3 + i][1] + j];                                                                                                                                                \n"
"        }                                                                                                                                                                                                  \n"
"                                                                                                                                                                                                           \n"
"        for( int j = 0; j < 3; j++ ) {																		\n"
"            vertices3[j][i] = (vertex[j]/vertex[3]);                                                                                                                                                       \n"
"		     norms[j][i] = norms_buff[3 * (faces[GROUP_ID * 3 + i][2]) + j];                                                                                                                                          \n"
"        }                                                                                                                                                                                                  \n"
"                                                                                                                                                                                                           \n"
"        if (   vertices3[0][i] > 0 && vertices3[0][i] < *screen_width                                                                                                                                      \n"
"            && vertices3[1][i] > 0 && vertices3[1][i] < *screen_width   )                                                                                                                                  \n"
"                out = false;                                                                                                                                                                               \n"
"    }                                                                                                                                                                                                      \n"
"                                                                                                                                                                                                           \n"
"    if(out) return;                                                                                                                                                                                        \n"
"                                                                                                                                                                                                           \n"
"    //if(vertices3[1][0] == vertices3[1][1] && vertices3[1][2] == vertices3[1][1]) return;                                                                                                                   \n"
"                                                                                                                                                                                                           \n"
"    int2 bounding_box_min = (int2) { *screen_width - 1, *screen_width - 1 };                                                                                                                                    \n"
"    int2 bounding_box_max = (int2) { 0, 0 };                                                                                                                                                                    \n"
"    int2 clamper = (int2) { *screen_width - 1, *screen_width - 1 };                                                                                                                                               \n"
"                                                                                                                                                                                                           \n"
"    for(int i = 0; i < 3; i++) {                                                                                                                                                                           \n"
"        for(int j = 0; j < 2; j++) {                                                                                                                                                                       \n"
"            bounding_box_min[j] = max(0, min(bounding_box_min[j], (int)vertices3[j][i]));                                                                                                                  \n"
"            bounding_box_max[j] = min(clamper[j], max(bounding_box_max[j], (int)vertices3[j][i]));                                                                                                           \n"
"        }                                                                                                                                                                                                  \n"
"    }                                                                                                                                                                                                      \n"
"                                                                                                                                                                                                           \n"
"    if(bounding_box_min[0] > *screen_width || bounding_box_max[0] < 0 || bounding_box_min[1] > *screen_width || bounding_box_max[1] < 0) return;                                                           \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"    int X_PER_ITEM = (int)(ceil((float)(bounding_box_max[0] - bounding_box_min[0]) / 16.f));                                                                                                                                                                                            \n"
"    int Y_PER_ITEM = (int)(ceil((float)(bounding_box_max[1] - bounding_box_min[1]) / 16.f));                                                                                                                                                                                            \n"
"    float STARTING_X = bounding_box_min[0] + X_PER_ITEM * (LOCAL_ID % 16);                                                                                                                                                                                            \n"
"    float ENDING_X = STARTING_X + X_PER_ITEM;                                                                                                                                                                                            \n"
"    float STARTING_Y = bounding_box_min[1] + Y_PER_ITEM * (LOCAL_ID / 16);                                                                                                                                                                                            \n"
"    float ENDING_Y = STARTING_Y + Y_PER_ITEM;                                                                                                                                                                                            \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"    float3 point;                                                                                                                                                                                            \n"
"    for(point[0] = STARTING_X; point[0] <= ENDING_X; point[0]++) {                                                                                                                     \n"
"        for(point[1] = STARTING_Y; point[1] <= ENDING_Y; point[1]++) {                                                                                                                 \n"
"           if(point[1] >= *screen_width || point[1] >= *screen_width) break;                                                                                                                                                                                                \n"
"			float3 bc_coord = barycentric(vertices3, point);                                                                                                                                                \n"
"           if (bc_coord[0] < 0 || bc_coord[1] < 0 || bc_coord[2] < 0) continue;                                                                                                                                                                                               \n"
"                                                                                                                                                                                                           \n"
"           float2 uv_vec = (float2){0, 0};                                                                                                                                                                                  \n"
"           float3 normal = (float3){0, 0, 0};                                                                                                                                                                \n"
"                                                                                                                                                                                                           \n"
"           point[2] = dot(vertices3[2], bc_coord);                                                                                                                                                                                   \n"
"			if (z_buffer[(int)(point[0] + point[1] * *screen_width)] > point[2]) {                                                                        \n"
"               continue;                                                                                                                                                                                   \n"
"           }                                                                                                                                                                                               \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"			for (int i = 0; i < 3; i++) {                                                                                                                                                                   \n"
"               uv_vec[0]  += uv_coords[i][0] * bc_coord[i];                                                                                                                                                 \n"
"               uv_vec[1]  += uv_coords[i][1] * bc_coord[i];                                                                                                                                                 \n"
"               normal[i]  = dot(norms[i], bc_coord);                                                                                                                                                 \n"
"           }                                                                                                                                                                                               \n"
"           int2 uv_point = (int2) { (int)(uv_vec[0] * map_size[0]), (int)(uv_vec[1] * map_size[1]) };                                                                                                                    \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"           int col_index = 3 * (uv_point[0] + uv_point[1] * map_size[0]);                                                                                                                                       \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"           float3 normalized_norm = normalize(normal);                                                                                                                                                                                                \n"
"                                                                                                                                                                                                           \n"
"                                                                                                                                                                                                           \n"
"           float intensity = clamp((dot(normalized_norm , (float3){light_dir[0], light_dir[1], light_dir[2]})), 0.f, 1.f) + 0.2;                                                                                                                                                                                                \n"
"                                                                                                                                                                                                           \n"
"           int color = 0;                                                                                                                                                                                  \n"
"           color |= ((int)fmin((float)(diffuse_map[col_index + 0]) * intensity, (float) 0xff)) << 16;                                                                                                                                                               \n"
"           color |= ((int)fmin((float)(diffuse_map[col_index + 1]) * intensity, (float) 0xff)) <<  8;                                                                                                                                                               \n"
"           color |= ((int)fmin((float)(diffuse_map[col_index + 2]) * intensity, (float) 0xff)) <<  0;                                                                                                                                                               \n"
"                                                                                                                                                                                                          \n"
"           //color |= ((int)fmin((float)(0xff) * intensity, (float) 0xff)) << 16;                                                                                                                                                               \n"
"           //color |= ((int)fmin((float)(0xff) * intensity, (float) 0xff)) <<  8;                                                                                                                                                               \n"
"           //color |= ((int)fmin((float)(0xff) * intensity, (float) 0xff)) <<  0;                                                                                                                                                               \n"
"                                                                                                                                                                                                          \n"
"                                                                                                                                                                                                          \n"
"           z_buffer[(int) (point[0] + point[1] * *screen_width)] = point[2];                                                                                                                                 \n"
"			pixels  [(int) (point[0] + point[1] * *screen_width)] = color;// & 0x00ffffff;                                                                                                                                     \n"
"        }                                                                                                                                                                                                  \n"
"    }                                                                                                                                                                                                      \n"
"}                                                                                                                                                                                                          \n";
