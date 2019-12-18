#pragma once
#include <vector>
#include "geometry.h"

class GeometryLibrary
{
	std::vector<Geometry>    m_Geometries;

public:
	GeometryLibrary() = default;
	GeometryLibrary(tinyxml2::XMLElement * xml);
};

class VisualSceneLibrary
{
	Visualscene    m_visual_scene;

public:
	VisualSceneLibrary() = default;
	VisualSceneLibrary(tinyxml2::XMLElement* xml);
};

