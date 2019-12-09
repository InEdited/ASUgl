#ifndef __CAMERA_HEADER__
#define __CAMERA_HEADER__
#include "geometry.h"

class Camera {
private:
	Vec3f position;
	Vec3f rotation;
	Vec3f up;
	Vec3f right;
	Vec3f forward;
	float fov;
	float near_plane;
	float far_plane;
	float horizontal_camera_speed;
	float vertical_camera_speed;
	float vertical_camera_clamp_up;
	float vertical_camera_clamp_down;
	float movement_speed;

public:
	Camera();
	void SetPosition(Vec3f pos);
	void SetRotation(Vec3f rot);
	void Move(Vec3f move_vec);
	void Rotate(Vec3f rot_vec);
	void SetFOV(int angle);
	void SetNearPlane(float near_val);
	void SetFarPlane(float far_val);
	void SetVerticalRotSpeed(float speed);
	void SetHorizontalRotSpeed(float speed);
	void SetClampRotUp(float angle);
	void SetClampRotDown(float angle);
	void SetMovementSpeed(float speed);
	Vec3f GetForward();

	void rotate_hor(float d_angle);
	void rotate_ver(float d_angle);
	void move_camera_left();
	void move_camera_right();
	void move_camera_forward();
	void move_camera_backward();

	void ApplyChanges();
	Matrix GetModelViewMatrix();
	Matrix GetProjectionMatrix();
};
#endif