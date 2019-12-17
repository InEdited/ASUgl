#include "jointtransform.h"



JointTransform::JointTransform(Vec3f position, Quaternion rotation)
{
	position_ = position;
	rotation_ = rotation;
}

Matrix JointTransform::getlocationtransform()
{
	Matrix matrix;
	Vec4f column = Vec4f(position_.x, position_.y, position_.z, 1);
	matrix.set_col(3, column);
	matrix = matrix * rotation_.toRotationMatrix();
	return matrix;
}

JointTransform JointTransform::interpolate(JointTransform frame1, JointTransform frame2, float progression)
{
	Vec3f pos = interpolate(frame1.position_, frame2.position_, progression);
	Quaternion rot = Quaternion::interpolate(frame1.rotation_, frame2.rotation_, progression);
	return JointTransform(pos, rot);
}

Vec3f JointTransform::interpolate(Vec3f start, Vec3f end, float progression)
{
	float x = start.x + (end.x - start.x) * progression;
	float y = start.y + (end.y - start.y) * progression;
	float z = start.z + (end.z - start.z) * progression;

	return Vec3f(x,y,z);
}
