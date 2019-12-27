#include "model.h"
#include "renderer.h"
#include "util_window.h"
#include "camera.h"
#include "util_renderer.h"
#include "CL/cl.h"
#include "kernels.h"
#include "improv_gfx.h"
#include <ctime>

#pragma comment (lib, "x86_64/opencl.lib")

#define HORIZONTAL_CAMERA_SPEED           0.1
#define VERTICAL_CAMERA_SPEED             0.1
#define VERTICAL_CAMERA_CLAMP_UP           90
#define VERTICAL_CAMERA_CLAMP_DOWN        -90
#define NEAR_CLIP_PLANE                    1.f 
#define FAR_CLIP_PLANE                     2000.0f
#define FOV                                50
#define CAMERA_MOVEMENT_SPEED              .7f
#define DEFAULT_CAMERA_POS     Vec3f(0, 0, 5)
#define DEFAULT_CAMERA_ROT     Vec3f(0, 0, 0)
#define LIGHT_INTENSITY                   1.5

Matrix ViewPort = Matrix::identity();
Matrix ModelView = Matrix::identity();
Matrix Projection = Matrix::identity();

Model* model, *model2;
Camera camera;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();
bool init_flag = false;

void init_camera() {
	camera.SetPosition(DEFAULT_CAMERA_POS);
	camera.SetRotation(DEFAULT_CAMERA_ROT);
	camera.SetFOV(FOV);
	camera.SetNearPlane(NEAR_CLIP_PLANE);
	camera.SetFarPlane(FAR_CLIP_PLANE);
	camera.SetClampRotDown(VERTICAL_CAMERA_CLAMP_DOWN);
	camera.SetClampRotUp(VERTICAL_CAMERA_CLAMP_UP);
	camera.SetHorizontalRotSpeed(HORIZONTAL_CAMERA_SPEED);
	camera.SetVerticalRotSpeed(VERTICAL_CAMERA_SPEED);
	camera.SetMovementSpeed(CAMERA_MOVEMENT_SPEED);
	camera.ApplyChanges();
}


std::vector<Model*> models_in_scene;


void render()
{
	if (!init_flag) {
		viewport(0, 0, screen_width, screen_height, FAR_CLIP_PLANE, NEAR_CLIP_PLANE);
		Setup();
		init_flag = true;
	}

	{
		Projection = camera.GetProjectionMatrix();
		ModelView = camera.GetModelViewMatrix();
	}

	Matrix z = ViewPort * Projection * ModelView;

	new_frame();
	for(Model* model : models_in_scene)
		model->render(&z, (float*)&light_dir);
	end_frame();

}

void AddModel(Model* model) {
	models_in_scene.push_back(model);
}

void free_renderer() {
	for(Model* model : models_in_scene)
		delete model;
}
