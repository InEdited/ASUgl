#include <iostream>
#include <fstream>
#include <sstream>
#include "ColladaModel.h"

#define PI 3.14159265358979323846
#define DEG2RAD PI/180

ColladaModel::ColladaModel(const char* filename) : faces_(), vertices_(), normals_()
{
	Transform = Matrix::identity();
	Rotation = Matrix::identity();
	Scale = Matrix::identity();
	Translation = Matrix::identity();

	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename);
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////faces//////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	face_count = 0;
	tinyxml2::XMLElement* xml_face = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("triangles");
	xml_face->QueryIntAttribute("count", &face_count); //get count
	std::stringstream str_triangle(xml_face->FirstChildElement("p")->GetText()); //get values as a string

	for (int i = 0; i < face_count; i++)
	{
		faces_.push_back(std::vector<Vec3i>());
		for (int j = 0; j < 3; j++)
		{
			Vec3i temp;
			str_triangle >> temp.x;
			str_triangle >> temp.y;
			str_triangle >> temp.z;
			faces_[i].push_back(temp);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////vertex/////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	vertex_count = 0;
	tinyxml2::XMLElement* xml_vertex = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->FirstChildElement("float_array");
	xml_vertex->QueryIntAttribute("count", &vertex_count);
	std::stringstream str_vertex(xml_vertex->GetText());

	for (int i = 0; i < vertex_count / 3; i++)
	{
		Vec3f temp;
		str_vertex >> temp.x;
		str_vertex >> temp.y;
		str_vertex >> temp.z;
		vertices_.push_back(temp);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////normal////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	normal_count = 0;
	tinyxml2::XMLElement* xml_normal = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->NextSiblingElement()->FirstChildElement("float_array");
	xml_normal->QueryIntAttribute("count", &normal_count);
	std::stringstream str_normal(xml_normal->GetText());

	for (int i = 0; i < normal_count / 3; i++)
	{
		Vec3f temp;
		str_normal >> temp.x;
		str_normal >> temp.y;
		str_normal >> temp.z;
		normals_.push_back(temp);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	texcoord_count = 0;
	tinyxml2::XMLElement* xml_texture = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->NextSiblingElement()->NextSiblingElement()->FirstChildElement("float_array");
	xml_texture->QueryIntAttribute("count", &texcoord_count);
	std::stringstream str_texcoord(xml_texture->GetText());

	for (int i = 0; i < texcoord_count / 2; i++)
	{
		Vec2f temp;
		str_texcoord >> temp.x;
		str_texcoord >> temp.y;
		textureco_.push_back(temp);
	}

	load_texture(filename, "_diffuse.tga", diffusemap_);
	load_texture(filename, "_nm_tangent.tga", normalmap_);
}

ColladaModel::~ColladaModel() {
}

void ColladaModel::ApplyTransform() {
	Transform = Translation * Scale * Rotation;
}

void ColladaModel::translate(Vec3f tr) {
	Translation[0][3] += tr.x;
	Translation[1][3] += tr.y;
	Translation[2][3] += tr.z;
}
void ColladaModel::rotate(Vec3f rot) {
	rot = rot * DEG2RAD;

	Rotation[0][0] = cosf(rot.y) * cosf(rot.z);
	Rotation[0][1] = -cosf(rot.y) * sinf(rot.z);
	Rotation[0][2] = sinf(rot.y);
	Rotation[1][0] = sinf(rot.x) * sinf(rot.y) * cosf(rot.z) + cosf(rot.x) * sinf(rot.z);
	Rotation[1][1] = -sinf(rot.x) * sinf(rot.y) * sinf(rot.z) + cosf(rot.x) * cosf(rot.z);
	Rotation[1][2] = -sinf(rot.x) * cosf(rot.y);
	Rotation[2][0] = -cosf(rot.x) * sinf(rot.y) * cosf(rot.z) + sinf(rot.x) * sinf(rot.z);
	Rotation[2][1] = cosf(rot.x) * sinf(rot.y) * sinf(rot.z) + sinf(rot.x) * cosf(rot.z);
	Rotation[2][2] = cosf(rot.x) * cosf(rot.y);
}
void ColladaModel::scale(Vec3f scl) {
	Scale[0][0] = scl.x;
	Scale[1][1] = scl.y;
	Scale[2][2] = scl.z;
}

int ColladaModel::nfaces() {
	return face_count;
}
int ColladaModel::nvertices() {
	return vertex_count;
}

std::vector<int> ColladaModel::face(int idx) {
	std::vector<int> face;
	for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
	return face;
}

Vec3f ColladaModel::vertix(int i) {
	return vertices_[i];
}

Vec3f ColladaModel::vertix(int iface, int nthvert) {
	return vertices_[faces_[iface][nthvert][0]];
}

void ColladaModel::load_texture(std::string filename, const char* suffix, TGAImage& img) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}

TGAColor ColladaModel::diffuse(Vec2f uvf) {
	Vec2i uv(uvf[0] * diffusemap_.get_width(), uvf[1] * diffusemap_.get_height());
	return diffusemap_.get(uv[0], uv[1]);
}

Vec3f ColladaModel::normal(Vec2f uvf) {
	Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
	TGAColor c = normalmap_.get(uv[0], uv[1]);
	Vec3f res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
	return res;
}

Vec2f ColladaModel::uv(int iface, int nthvert) {
	return textureco_[faces_[iface][nthvert][2]];
}

float ColladaModel::specular(Vec2f uvf) {
	Vec2i uv(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height());
	return specularmap_.get(uv[0], uv[1])[0] / 1.f;
}

Vec3f ColladaModel::normal(int iface, int nthvert) {
	int idx = faces_[iface][nthvert][1];
	return normals_[idx].normalize();
}
