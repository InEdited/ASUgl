#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "tinyxml2.h"
//----------------------------------------------//
class Float_Array
{
	unsigned int        m_Count;
	std::string              m_ID;
	float* m_Floats;

public:
	Float_Array() = default;
	Float_Array(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Name_Array
{
	unsigned int        m_Count;
	std::string              m_ID;
	std::string* m_Names;
public:
	Name_Array() = default;
	Name_Array(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Param
{
	std::string              m_Name;
	std::string              m_Type;
public:
	Param() = default;
	Param(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Accessor
{
	std::string              m_Source;
	unsigned int        m_Count;
	unsigned int        m_Stride;
	std::vector<Param>       m_Params;
public:
	Accessor () = default;
	Accessor(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Technique_Common
{
	Accessor            m_Accessor;
public:
	Technique_Common() = default;
	Technique_Common(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Source
{
	std::string              m_ID;
	Float_Array         m_Float_Array;
	Name_Array          m_Name_Array;
	Technique_Common    m_TechniqueCommon;
public:
	Source() = default;
	Source(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Input
{
	std::string              m_Semantic;
	std::string              m_Source;
	std::string              m_Offset;
	std::string              m_Set;
public:
	Input() = default;
	Input(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Vertices
{
	std::string              m_ID;
	std::vector<Input>       m_Inputs;
public:
	Vertices() = default;
	Vertices(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class P
{
	unsigned int         m_Count;
	unsigned short* m_Indices;
public:
	P() = default;
	P(tinyxml2::XMLElement* xml, unsigned int count);
};
//----------------------------------------------//
class Triangles
{
	unsigned int        m_Count;
	std::string              m_Material;
	std::vector<Input>       m_Inputs;
	P                   m_P;
public:
	Triangles() = default;
	Triangles(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Mesh
{
	std::vector<Source>      m_Sources;
	Vertices            m_Vertices;
	std::vector<Triangles>   m_Triangles;
public:
	Mesh() = default;
	Mesh(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Node
{
	std::string      m_ID;
	std::string      m_name;
	std::string      m_type;
	std::string      m_sid;
public:
	Node() = default;
	Node(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
class Matrix
{	
	std::string      m_sid;
public:
	Node() = default;
	Node(tinyxml2::XMLElement* xml);
};
//----------------------------------------------//
