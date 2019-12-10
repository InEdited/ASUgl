#include "ColladaModel.h"

ColladaModel::ColladaModel(const char* filename) : faces_(), vertices_(), normals_()
{
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
}

ColladaModel::~ColladaModel() {
}

std::vector<Vec3i> ColladaModel::face(int idx) {
	return faces_[idx];
}

Vec3f ColladaModel::vertex(int i) {
	return vertices_[i];
}

int ColladaModel::nvertices() {
	return vertex_count;
}

int ColladaModel::nfaces() {
	return face_count;
}