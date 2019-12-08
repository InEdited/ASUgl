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
	std::vector<Vec3f> positions_;
	std::vector<Vec3f> normals_;
	std::vector<std::vector<Vec3i> > triangles_;

	int vertice_count;
	int triangle_count;
public:
	ColladaModel(const char* filename);
	~ColladaModel();

	Vec3f position(int i);
	std::vector<Vec3i> triangle(int idx);

	int nposition();
	int ntriangle();
};

#endif