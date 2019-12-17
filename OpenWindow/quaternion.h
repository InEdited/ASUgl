#pragma once
#include "geometry.h"
class Quaternion
{
private:
	float x_, y_, z_, w_;


public:
	Quaternion() = default;
	Quaternion(float x, float y, float z, float w);
	void normalize();
	Matrix toRotationMatrix();
	static Quaternion fromMatrix(Matrix matrix);
	static Quaternion interpolate(Quaternion a, Quaternion b, float blend);
};

