#include "model.h"
#include "renderer.h"
#include "util_window.h"
#include "camera.h"
#include "util_renderer.h"
#include "CL/cl.h"
#include "kernels.h"
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

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Matrix ViewPort = Matrix::identity();
Matrix ModelView = Matrix::identity();
Matrix Projection = Matrix::identity();

Model* model = new Model("sakura.obj");
Camera camera;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();

float* new_verts = (float*)malloc(4 * sizeof(float) * model->nverts());
cl_int3* faces = (cl_int3*)malloc(3 * sizeof(cl_int3) * model->nfaces());

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

void clear_zbuffer()
{
	for (int i = 0; i < screen_width * screen_height; i++)
		z_buffer[i] = 0;
}


struct TextureShader : public IShader {
	mat<2, 3, float> varying_uv_coords;
	Matrix uniform_mit;
	Matrix uniform_m;
	Matrix z;

	virtual Vec4f vertex(int iface, int nthvert) {
		//varying_uv_coords.set_col(nthvert, model->vert(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->verts_[model->faces_[iface][nthvert][0]]);
		//return ViewPort * Projection * ModelView * gl_Vertex;									// transform it to screen coordinates
		return z * gl_Vertex;
		//return Vec4f(0,0,0,0);
	}

	virtual bool fragment(Vec3f bar, TGAColor &color) {
		Vec2f uv = varying_uv_coords * bar;
		Vec3f normal = Vec3f(uniform_mit * Vec4f(model->normal(uv))).normalize();
		Vec3f light =  Vec3f(uniform_m * Vec4f(light_dir)).normalize();
		Vec3f reflection = (normal * (normal*light*2.f) - light).normalize();
		float spec_intensity = pow(std::fmax(reflection.z, 0.f), model->specular(uv));
		float diff_intensity = std::fmax(0.f, (normal*light));
		TGAColor c = model->diffuse(uv);
		color = c;
		for (int i = 0; i < 3; i++)
			color[i] = std::fmin(3 + c[i] * ((1 * diff_intensity + 0.1 * spec_intensity)), 255);// *LIGHT_INTENSITY;
		return false;                         
	}
};


void render()
{
	if (!init_flag) {
		//light_dir = camera.GetForward().normalize() * -1;
		viewport(0, 0, screen_width, screen_height, FAR_CLIP_PLANE, NEAR_CLIP_PLANE);
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				faces[i * 3 + j].x = model->faces_[i][j][0];
				faces[i * 3 + j].y = model->faces_[i][j][1];
				faces[i * 3 + j].z = model->faces_[i][j][2];
			}
		}
		init_flag = true;
	}

	{
		Projection = camera.GetProjectionMatrix();
		ModelView = camera.GetModelViewMatrix();
	}
	
	{
		//model->rotate(Vec3f(0, 0, 90));
		//model->ApplyTransform();
	}

//	float* normal_test = *(float**)((Vec3f*)&model->norms_);
//
//	Vec3f smth = Vec3f(
//		normal_test[3 * faces[0 * 9 + 0 * 3 + 2] + 0],
//		normal_test[3 * faces[0 * 9 + 0 * 3 + 2] + 1],
//		normal_test[3 * faces[0 * 9 + 0 * 3 + 2] + 2]
//	).normalize();
//	printf("Real: %f, %f, %f\n", model->normal(0, 0)[0], model->normal(0, 0)[1], model->normal(0, 0)[2]);
//	printf("Please: %f, %f, %f\n", smth[0], smth[1], smth[2]);

	//TextureShader shader;
	//shader.uniform_m =   (Projection);
	//shader.uniform_mit = (Projection).invert_transpose();

	Matrix z = ViewPort * Projection * ModelView * model->Transform;

	Matrix uniform_m =   (Projection);
	Matrix uniform_mit = (Projection).invert_transpose();

	// Vertex Shader: Should be called per model
	vertex_shader((float*)&z, *(float**)((Vec3f*) &model->verts_), model->nverts(), new_verts);

	// Things needed in the GPU fragment shader
	// [x] model->faces_
	// [x] model->nfaces()
	// [x] new_verts
	// [x] model->nverts()
	// [x] screen_width
	// [x] screen_height
	// [x] z_buffer
	// [x] uniform_m
	// [x] uniform_mit
	// [x] light direction
	// [ ] diffuse map
	// [ ] normal map
	// [ ] specular map
	// [ ] pixel_data

	int map_size[] = {model->diffusemap_.get_width(), model->diffusemap_.get_height()};

	fragment_shader(
		faces,
		model->nfaces(),
		*(float**)((Vec2f*)&model->uv_),
		sizeof(float) * model->uv_.size() * 2,
		(float*) &uniform_m,
		(float*) &uniform_mit,
		(float*) &light_dir,
		model->diffusemap_.data,
		*(float**)((Vec3f*)&model->norms_),
		sizeof(float) * model->norms_.size() * 3,
		model->specularmap_.data,
		map_size
	);

	// Here starts the CPU fragment shader
	//printf("Here starts the loop\n");
	//#pragma omp parallel for
	//for (int i = 0; i < model->nfaces(); i++) {
	//	Vec4f screen_coords[3];
	//	bool out = true;
	//	#pragma omp parallel for
	//	for (int j = 0; j < 3; j++) {
	//		screen_coords[j] = ((Vec4f*)new_verts)[model->faces_[i][j][0]];
	//		Vec3f screen3(screen_coords[j]);

	//		shader.varying_uv_coords.set_col(j, model->uv(i, j));
	//		if (screen3.x > 0 && screen3.x < screen_width && screen3.y > 0 && screen3.y < screen_height) out = false;
	//	}
	//	if(!out)
	//		triangle(screen_coords, shader);
	//}

	//printf("that's it\n");
}

