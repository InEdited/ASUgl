#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"
#include "kernels.h"
#include "util_window.h"

class Model {
private:
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i> > faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
	cl_program vertex_shader_prog;
	cl_program fragment_shader_prog;
	cl_kernel vertex_shader_kernel;
	cl_kernel fragment_shader_kernel;
	cl_mem vertex_shader_matz;
	cl_mem vertex_shader_vertices;
	cl_mem new_vertices_mem;
	cl_mem fragment_shader_faces;
	cl_mem fragment_shader_screen_width;
	cl_mem fragment_shader_uv;
	cl_mem fragment_shader_map_size;
	cl_mem fragment_shader_norms;
	cl_mem fragment_shader_light_dir;
	cl_mem fragment_shader_diffuse_map;
	cl_int3* faces;

    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f normal(int iface, int nthvert);
    Vec3f normal(Vec2f uv);
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
	Matrix Transform;
	Matrix Rotation;
	Matrix Scale;
	Matrix Translation;
	void translate(Vec3f tr);
	void rotate(Vec3f rot);
	void scale(Vec3f scl);
	void ApplyTransform();
	void init_kernels();
	void vertex(float* z);
	void fragment(float* light_dir);
	void render(Matrix* z, float* light_dir);
	void release_kernels();
    TGAColor diffuse(Vec2f uv);
    float specular(Vec2f uv);
    std::vector<int> face(int idx);
};
#endif //__MODEL_H__

