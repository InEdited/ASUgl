#include "Joint.h"

Joint::Joint(int index, Matrix transform, char* name) {
	Joint::index = index;
	_transform = transform;
	Joint::name = name;
}

Joint::~Joint() {
}

void Joint::addChild(Joint child) {
	children.push_back(child);
}

Matrix Joint::getTransform() {
	return _transform;
}

void Joint::setTransform(Matrix transform) {
	_transform = transform;
}

Matrix Joint::getInverseBindTransform() {
	return _inverseBindtransform;
}

void Joint::calculateInverseBindTransform(Matrix parentBindTransform) {
	Matrix bindTransform = parentBindTransform * _localBindTransform;
	_inverseBindtransform = bindTransform.invert();

	for (auto &child : children)
	{
		child.calculateInverseBindTransform(bindTransform);
	}
}