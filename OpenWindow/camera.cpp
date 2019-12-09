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


void Camera::rotate_camera_right() { 
	rotation.y += horizontal_camera_speed; 
}
void Camera::rotate_camera_left() { 
	rotation.y -= horizontal_camera_speed; 
}
void Camera::rotate_camera_up() {
	rotation.x += vertical_camera_speed;
	if (rotation.x > vertical_camera_clamp_up) rotation.x = 90;
}
void Camera::rotate_camera_down() {
	rotation.x -= vertical_camera_speed;
	if (rotation.x < vertical_camera_clamp_down) rotation.x = -90;
}

#define MOVEMENT_SPEED 0.05f;

void Camera::move_camera_left() { 
	position = position - right * MOVEMENT_SPEED;
}
void Camera::move_camera_right() { 
	position = position + right * MOVEMENT_SPEED;
}
void Camera::move_camera_forward() {
	position = position + forward * MOVEMENT_SPEED;
}
void Camera::move_camera_backward() {
	position = position - forward * MOVEMENT_SPEED;
}

void Camera::ApplyChanges() {
	forward = Vec3f(sin(rotation.y * DEG2RAD), -sin(rotation.x * DEG2RAD), -cosf(rotation.y*DEG2RAD) * cosf(rotation.x*DEG2RAD));
	right = Vec3f(cos(rotation.y*DEG2RAD), 0, sin(rotation.y * DEG2RAD));
	up = cross(right, forward);
}

Matrix Camera::GetModelViewMatrix() {
	Vec3f center = position + forward;
	Vec3f z = (position - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix Minv = Matrix::identity();
	Matrix Tr = Matrix::identity();
	for (int i = 0; i < 3; i++) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
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
