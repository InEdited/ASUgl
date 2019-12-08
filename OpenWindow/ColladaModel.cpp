#include "ColladaModel.h"



string trim(const string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (string::npos == first)
	{
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

ColladaModel::ColladaModel(const char* filename) : positions_(), triangles_() ,normals_()
{
	XMLDocument doc;
	doc.LoadFile("resources/face.dae");

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	XMLElement* xml_triangle_count = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("triangles");
	int triangle_count = 0;
	xml_triangle_count->QueryIntAttribute("count", &triangle_count);

	XMLElement* xml_triangle = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("triangles")->FirstChildElement("p");
	const char* xml2_triangle = xml_triangle->GetText();
	stringstream str_triangle(xml2_triangle);

	vector<vector<Vec3i>> triangle(triangle_count, vector<Vec3i>(3, Vec3i(0, 0, 0)));
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
	XMLElement* xml_vertice = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->FirstChildElement("float_array");
	int vertice_count = 0;
	xml_vertice->QueryIntAttribute("count", &vertice_count);

	const char* xml2_vertice = xml_vertice->GetText();
	stringstream str_vertice(xml2_vertice);

	vector<Vec3f> vertice(vertice_count / 3);
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
	XMLElement* xml_normal_count = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->NextSiblingElement()->FirstChildElement("float_array");
	int normal_count = 0;
	xml_normal_count->QueryIntAttribute("count", &normal_count);
	XMLElement* xml_normal = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh")->FirstChildElement("source")->NextSiblingElement()->FirstChildElement("float_array");
	const char* xml2_normal = xml_normal->GetText();
	stringstream str_normal(xml2_normal);

	vector<Vec3f> normal(normal_count / 3);
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

std::vector<int> ColladaModel::triangle(int idx) {
	return triangles_[idx];
}

Vec3f ColladaModel::position(int i) {
	return positions_[i];
}