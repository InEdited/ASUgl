#include "camera.h"

Camera::Camera()
{
	fov = 30;
	position = Vec3f(0, 0, 0);
	rotation = Vec3f(0, 0, 0);
	up = Vec3f(0, 0, 0);
	right = Vec3f(0, 0, 0);
	forward = Vec3f(0, 0, 0);

	near_plane = 0;
	far_plane = 15;
	horizontal_camera_speed = 0.5;
	vertical_camera_speed = 0.5;
	vertical_camera_clamp_up = 90;
	vertical_camera_clamp_down = -90;
	movement_speed = 1.f;
}

Vec3f Camera::GetForward() {
	return forward;
}

void Camera::Move(Vec3f move_vec) { 
	position = position + move_vec; 
}

void Camera::SetPosition(Vec3f pos) { 
	position = pos; 
}
void Camera::Rotate(Vec3f rot_vec) { 
	rotation = rotation + rot_vec; 
}
void Camera::SetRotation(Vec3f rot) { 
	rotation = rot; 
}
void Camera::SetFOV(int angle) { 
	fov = angle; 
}
void Camera::SetVerticalRotSpeed(float speed) {
	vertical_camera_speed = speed;
}
void Camera::SetHorizontalRotSpeed(float speed) {
	horizontal_camera_speed = speed;
}
void Camera::SetClampRotUp(float angle) {
	vertical_camera_clamp_up = angle;
}
void Camera::SetClampRotDown(float angle) {
	vertical_camera_clamp_down = angle;
}
void Camera::SetNearPlane(float near_val) { 
	near_plane = near_val; 
}
void Camera::SetFarPlane(float far_val) { 
	far_plane = far_val; 
}

void Camera::rotate_hor(float d_angle) {
	rotation.y += d_angle * horizontal_camera_speed;
}
void Camera::rotate_ver(float d_angle) {
	rotation.x += d_angle * vertical_camera_speed;
	rotation.x = std::fmin(rotation.x, vertical_camera_clamp_up);
	rotation.x = std::fmax(rotation.x, vertical_camera_clamp_down);
}


void Camera::move_camera_left() { 
	position = position - right * movement_speed;
}
void Camera::move_camera_right() { 
	position = position + right * movement_speed;
}
void Camera::move_camera_forward() {
	position = position + forward * movement_speed;
}
void Camera::move_camera_backward() {
	position = position - forward * movement_speed;
}
void Camera::rise() {
	position = position + Vec3f(0, movement_speed, 0);
}
void Camera::fall() {
	position = position - Vec3f(0, movement_speed, 0);
}

void Camera::SetMovementSpeed(float speed) {
	movement_speed = speed;
}

void Camera::ApplyChanges() {
	forward = Vec3f(sin(rotation.y * DEG2RAD), -sin(rotation.x * DEG2RAD), -cosf(rotation.y*DEG2RAD) * cosf(rotation.x*DEG2RAD)).normalize();
	right = Vec3f(cos(rotation.y*DEG2RAD), 0, sin(rotation.y * DEG2RAD)).normalize();
	up = cross(right, forward).normalize();
}

Matrix Camera::GetModelViewMatrix() {
	Vec3f center = position + forward;
	Vec3f z = forward * -1;
	Matrix Minv = Matrix::identity();
	Matrix Tr = Matrix::identity();
	for (int i = 0; i < 3; i++) {
		Minv[0][i] = right[i];
		Minv[1][i] = up[i];
		Minv[2][i] = z[i];
		Tr[i][3] = -center[i];
	}
	return Minv * Tr;
}

Matrix Camera::GetProjectionMatrix() {
	Matrix Projection = Matrix::identity();
	Projection[0][0] = 1 / tan(fov * DEG2RAD);
	Projection[1][1] = 1 / tan(fov * DEG2RAD);
	Projection[2][2] = (far_plane + near_plane) / (far_plane - near_plane);
	Projection[2][3] = (-2 * far_plane * near_plane) / (far_plane - near_plane);
	Projection[3][2] = -1;
	return Projection;
}
