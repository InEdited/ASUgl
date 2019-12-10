#ifndef __MODEL_s__
#define __MODEL_s__

#pragma once

#include "ColladaModel.h"
#include "tinyxml2.h"
#include "geometry.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>


class ColladaModel {
private:
	int face_count;
	int vertex_count;
	int normal_count;
	int textureco_count;

	std::vector<std::vector<Vec3i> > faces_;
	std::vector<Vec3f> vertices_;
	std::vector<Vec3f> normals_;
	std::vector<Vec3f> textureco_;
public:
	ColladaModel(const char* filename);
	~ColladaModel();

	std::vector<Vec3i> face(int idx);
	Vec3f vertex(int i);
	Vec3f normal(int i);
	Vec3f textureco(int i);

	int nfaces();
	int nvertices();
	int nnormals();
	int ntextureco();
};

#endif