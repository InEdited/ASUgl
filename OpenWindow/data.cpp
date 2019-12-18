#include "data.h"

Float_Array::Float_Array(tinyxml2::XMLElement* xml)
{
	xml->QueryIntAttribute("count", (int*)&m_Count);

	const char* ch;
	xml->QueryStringAttribute("count", &ch);
	std::string s(ch);
	m_ID = s;

	std::stringstream str(xml->GetText());
	m_Floats = new float[m_Count];

	for (int i = 0; i < m_Count; i++)
	{
		 str >> m_Floats[i];
	}
	float f = m_Floats[55358];
}

Name_Array::Name_Array(tinyxml2::XMLElement* xml) 
{
	xml->QueryIntAttribute("count", (int*)&m_Count);

	const char* ch;
	xml->QueryStringAttribute("count", &ch);
	std::string s(ch);
	m_ID = s;

	std::stringstream str(xml->GetText());
	for (int i = 0; i < m_Count; i++)
	{
		str >> m_Names[i];
	}
}

Param::Param(tinyxml2::XMLElement* xml) 
{
	const char* ch;
	xml->QueryStringAttribute("name", &ch);
	std::string s(ch);
	m_Name = s;

	const char* ch1;
	xml->QueryStringAttribute("type", &ch1);
	std::string s1(ch1);
	m_Type = s1;
}

Accessor::Accessor(tinyxml2::XMLElement* xml) 
{
	const char* ch;
	xml->QueryStringAttribute("source", &ch);
	std::string s(ch);
	m_Source = s;

	xml->QueryIntAttribute("count", (int*)&m_Count);

	xml->QueryIntAttribute("stride", (int*)&m_Stride);

	for (tinyxml2::XMLElement* child = xml->FirstChildElement("param"); child != NULL; child = child->NextSiblingElement("param"))
	{
		Param p(child);
		m_Params.push_back(p);
	}
}

Technique_Common::Technique_Common(tinyxml2::XMLElement* xml)
{
	m_Accessor = Accessor(xml->FirstChildElement("accessor"));
}

Source::Source(tinyxml2::XMLElement* xml)
{
	const char* ch;
	xml->QueryStringAttribute("id", &ch);
	std::string s(ch);
	m_ID = s;

	tinyxml2::XMLElement* fa = xml->FirstChildElement("float_array");
	if (fa != NULL)
	{
		m_Float_Array = Float_Array(fa);
	}

	tinyxml2::XMLElement* na = xml->FirstChildElement("Name_array");
	if (na != NULL)
	{
		m_Name_Array = Name_Array(na);
	}

	m_TechniqueCommon = Technique_Common(xml->FirstChildElement("technique_common"));
}

Input::Input(tinyxml2::XMLElement* xml)
{
	const char* ch;
	xml->QueryStringAttribute("semantic", &ch);
	std::string s(ch);
	m_Semantic = s;

	const char* ch1;
	xml->QueryStringAttribute("source", &ch1);
	std::string s1(ch1);
	m_Source = s1;
	
	if (xml->FindAttribute("offset"))
	{
		const char* ch2;
		xml->QueryStringAttribute("offset", &ch2);
		std::string s2(ch2);
		m_Offset = s2;
	}

	if (xml->FindAttribute("set"))
	{
		const char* ch3;
		xml->QueryStringAttribute("set", &ch3);
		std::string s3(ch3);
		m_Set = s3;
	}
}

Vertices::Vertices(tinyxml2::XMLElement* xml)
{
	const char* ch;
	xml->QueryStringAttribute("id", &ch);
	std::string s(ch);
	m_ID = s;

	for (tinyxml2::XMLElement* child = xml->FirstChildElement("input"); child != NULL; child = child->NextSiblingElement("input"))
	{
		Input in(child);
		m_Inputs.push_back(in);
	}
}

P::P(tinyxml2::XMLElement* xml, unsigned int count)
{
	m_Count = count;
	std::stringstream str(xml->GetText());
	m_Indices = new unsigned short[count * 9];
	for (int i = 0; i < count * 9; i++)
	{
		str >> m_Indices[i];
	}
}

Triangles::Triangles(tinyxml2::XMLElement* xml)
{
	xml->QueryIntAttribute("count", (int*)&m_Count);

	const char* ch;
	xml->QueryStringAttribute("count", &ch);
	std::string s(ch);
	m_Material = s;

	for (tinyxml2::XMLElement* child = xml->FirstChildElement("input"); child != NULL; child = child->NextSiblingElement("input"))
	{
		Input in(child);
		m_Inputs.push_back(in);
	}

	tinyxml2::XMLElement* mp = xml->FirstChildElement("p");
	if (mp != NULL)
	{
		m_P = P(mp, m_Count);
	}
}

Mesh::Mesh(tinyxml2::XMLElement* xml)
{
	for (tinyxml2::XMLElement* child = xml->FirstChildElement("source"); child != NULL; child = child->NextSiblingElement("source"))
	{
		Source sc(child);
		m_Sources.push_back(sc);
	}

	m_Vertices = Vertices(xml->FirstChildElement("vertices"));

	for (tinyxml2::XMLElement* child = xml->FirstChildElement("triangles"); child != NULL; child = child->NextSiblingElement("triangles"))
	{
		Triangles tr(child);
		m_Triangles.push_back(tr);
	}
}