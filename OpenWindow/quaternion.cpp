#include "quaternion.h"
#include <math.h>

Quaternion::Quaternion(float x, float y, float z, float w)
{
	x_ = x;
	y_ = y;
	z_ = z;
	w_ = w;
	normalize();
}

void Quaternion::normalize()
{
	float mag = (float)sqrt(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_);
	w_ /= mag;
	x_ /= mag;
	y_ /= mag;
	z_ /= mag;
}

Matrix Quaternion::toRotationMatrix()
{
	Matrix matrix;
	float xy = x_ * y_;
	float xz = x_ * z_;
	float xw = x_ * w_;
	float yz = y_ * z_;
	float yw = y_ * w_;
	float zw = z_ * w_;
	float xSquared = x_ * x_;
	float ySquared = y_ * y_;
	float zSquared = z_ * z_;
	
	Vec4f c0 = Vec4f(1 - 2 * (ySquared + zSquared), 2 * (xy - zw), 2 * (xz + yw),0);
	Vec4f c1 = Vec4f(2 * (xy + zw), 1 - 2 * (xSquared + zSquared), 2 * (yz - xw), 0);
	Vec4f c2 = Vec4f(2 * (xz - yw), 2 * (yz + xw), 1 - 2 * (xSquared + ySquared), 0);
	Vec4f c3 = Vec4f(0,0,0,1);

	matrix.set_col(0,c0);
	matrix.set_col(1, c1);
	matrix.set_col(2, c2);
	matrix.set_col(3, c3);
	return matrix;
}

Quaternion Quaternion::fromMatrix(Matrix matrix)
{
	float w_, x_, y_, z_;
	float diagonal = matrix[0][0] + matrix[1][1] + matrix[2][2];
	if (diagonal > 0) {
		float w4 = (float)(sqrt(diagonal + 1.f) * 2.f);
		w_ = w4 / 4.f;
		x_ = (matrix[2][1] - matrix[1][2]) / w4;
		y_ = (matrix[0][2] - matrix[2][0]) / w4;
		z_ = (matrix[1][0] - matrix[0][1]) / w4;
	}
	else if ((matrix[0][0] > matrix[1][1]) && (matrix[0][0] > matrix[2][2])) {
		float x4 = (float)(sqrt(1.f + matrix[0][0] - matrix[1][1] - matrix[2][2]) * 2.f);
		w_ = (matrix[2][1] - matrix[1][2]) / x4;
		x_ = x4 / 4.f;
		y_ = (matrix[0][1] + matrix[1][0]) / x4;
		z_ = (matrix[0][2] + matrix[2][0]) / x4;
	}
	else if (matrix[1][1] > matrix[2][2]) {
		float y4 = (float)(sqrt(1.f + matrix[1][1] - matrix[0][0] - matrix[2][2]) * 2.f);
		w_ = (matrix[0][2] - matrix[2][0]) / y4;
		x_ = (matrix[0][1] + matrix[1][0]) / y4;
		y_ = y4 / 4.f;
		z_ = (matrix[1][2] + matrix[2][1]) / y4;
	}
	else {
		float z4 = (float)(sqrt(1.f + matrix[2][2] - matrix[0][0] - matrix[1][1]) * 2.f);
		w_ = (matrix[1][0] - matrix[0][1]) / z4;
		x_ = (matrix[0][2] + matrix[2][0]) / z4;
		y_ = (matrix[1][2] + matrix[2][1]) / z4;
		z_ = z4 / 4.f;
	}
	return Quaternion(x_, y_, z_, w_);
}

Quaternion Quaternion::interpolate(Quaternion a, Quaternion b, float blend)
 {
	Quaternion result(0, 0, 0, 1);
	float dot = a.w_ * b.w_ + a.x_ * b.x_ + a.y_ * b.y_ + a.z_ * b.z_;
	float blendI = 1.f - blend;
	if (dot < 0) {
		result.w_ = blendI * a.w_ + blend * -b.w_;
		result.x_ = blendI * a.x_ + blend * -b.x_;
		result.y_ = blendI * a.y_ + blend * -b.y_;
		result.z_ = blendI * a.z_ + blend * -b.z_;
	}
	else {
		result.w_ = blendI * a.w_ + blend * b.w_;
		result.x_ = blendI * a.x_ + blend * b.x_;
		result.y_ = blendI * a.y_ + blend * b.y_;
		result.z_ = blendI * a.z_ + blend * b.z_;
	}
	result.normalize();
	return result;
}



