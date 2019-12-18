#include "GeometryLibrary.h"

GeometryLibrary::GeometryLibrary(tinyxml2::XMLElement* xml)
{
	auto xm = xml;
	auto g = Geometry(xml->FirstChildElement("geometry"));
	m_Geometries.push_back(g);
}

