#pragma once
#include <vector>
#include "geometry.h"
class Joint
{
private:
	Matrix _transform;	//model space
	Matrix _localBindTransform;
	Matrix _inverseBindtransform;

public:
	int index;
	char* name;
	std::vector<Joint> children;

	Joint() = default;
	Joint(int index,Matrix transform,char* name);
	~Joint();

	void addChild(Joint child);

	Matrix getTransform();
	void setTransform(Matrix transform);

	Matrix getInverseBindTransform();
	void calculateInverseBindTransform(Matrix parentBindTransform);
};

