#include "ColladaModel.h"

ColladaModel::ColladaModel(const char* filename) : positions_(), triangles_() ,normals_()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	tinyxml2::XMLElement* xml_triangle_count = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("triangles");
	triangle_count = 0;
	xml_triangle_count->QueryIntAttribute("count", &triangle_count);

	tinyxml2::XMLElement* xml_triangle = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("triangles")->FirstChildElement("p");
	const char* xml2_triangle = xml_triangle->GetText();
	std::stringstream str_triangle(xml2_triangle);

	std::vector<std::vector<Vec3i>> triangle(triangle_count, std::vector<Vec3i>(3, Vec3i(0, 0, 0)));
	for (int i = 0; i < triangle_count; i++)
	{
		int x = 0;

		str_triangle >> x;
		triangle[i][0].x = x;

		str_triangle >> x;
		triangle[i][1].x = x;

		str_triangle >> x;
		triangle[i][2].x = x;



		str_triangle >> x;
		triangle[i][0].y = x;

		str_triangle >> x;
		triangle[i][1].y = x;

		str_triangle >> x;
		triangle[i][2].y = x;



		str_triangle >> x;
		triangle[i][0].z = x;

		str_triangle >> x;
		triangle[i][1].z = x;

		str_triangle >> x;
		triangle[i][2].z = x;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	tinyxml2::XMLElement* xml_vertice = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->FirstChildElement("float_array");
	vertice_count = 0;
	xml_vertice->QueryIntAttribute("count", &vertice_count);

	const char* xml2_vertice = xml_vertice->GetText();
	std::stringstream str_vertice(xml2_vertice);

	std::vector<Vec3f> vertice(vertice_count / 3);
	for (int i = 0; i < vertice_count / 3; i++)
	{
		float x = 0;

		str_vertice >> x;
		vertice[i].x = x;

		str_vertice >> x;
		vertice[i].y = x;

		str_vertice >> x;
		vertice[i].z = x;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	tinyxml2::XMLElement* xml_normal_count = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->NextSiblingElement()->FirstChildElement("float_array");
	int normal_count = 0;
	xml_normal_count->QueryIntAttribute("count", &normal_count);
	tinyxml2::XMLElement* xml_normal = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->NextSiblingElement()->FirstChildElement("float_array");
	const char* xml2_normal = xml_normal->GetText();
	std::stringstream str_normal(xml2_normal);

	std::vector<Vec3f> normal(normal_count / 3);
	for (int i = 0; i < normal_count / 3; i++)
	{
		float x = 0;

		str_normal >> x;
		normal[i].x = x;

		str_normal >> x;
		normal[i].y = x;

		str_normal >> x;
		normal[i].z = x;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
}

ColladaModel::~ColladaModel() {
}

std::vector<Vec3i> ColladaModel::triangle(int idx) {
	return triangles_[idx];
}

Vec3f ColladaModel::position(int i) {
	return positions_[i];
}

int ColladaModel::nposition() {
	return vertice_count;
}

int ColladaModel::ntriangle() {
	return triangle_count;
}