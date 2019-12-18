#include "COLLADA.h"

COLLADA::COLLADA(const char* filename)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename);
	tinyxml2::XMLElement* xml = doc.FirstChildElement("COLLADA");
	m_GeometryLibrary = GeometryLibrary(xml->FirstChildElement("library_geometries"));
}
