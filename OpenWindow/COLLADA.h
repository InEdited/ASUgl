#pragma once
#include "GeometryLibrary.h"
class COLLADA
{
private:
	GeometryLibrary     m_GeometryLibrary;
	//ImagesLibrary       m_ImagesLibrary;
	//EffectsLibrary      m_EffectsLibrary;
	//MaterialLibrary     m_MaterialLibrary;
	//ControllerLibrary   m_ControllerLibrary;
	//AnimationLibrary    m_AnimationLibrary;
	VisualSceneLibrary  m_VisualSceneLibrary;

public:
	COLLADA() = default;
	COLLADA(const char* filename);
};

