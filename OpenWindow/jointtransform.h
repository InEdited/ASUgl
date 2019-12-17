#pragma once

#include "quaternion.h"
#include "geometry.h"

class JointTransform
{
private:

	Vec3f position_;

	Quaternion rotation_;

public:
	JointTransform() = default;
	JointTransform(Vec3f position,Quaternion rotation);

	Matrix getlocationtransform();

	static JointTransform interpolate(JointTransform frame1, JointTransform frame2, float progression);

	static Vec3f interpolate(Vec3f start,Vec3f end, float progression);
};

