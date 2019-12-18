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
#define NEAR_CLIP_PLANE                    0.3f 
#define FAR_CLIP_PLANE                     10.0f
#define FOV                                30
#define CAMERA_MOVEMENT_SPEED           0.05f
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

Model* model = new Model("african_head.obj");
Camera camera;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();

float* new_verts = (float*)malloc(4 * sizeof(float) * model->nverts());

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
			color[i] = std::fmin(2 + c[i] * (( 1 * diff_intensity + 1 * spec_intensity)), 255) * LIGHT_INTENSITY;
		return false;                         
	}
};


void render()
{
	{
		//light_dir = camera.GetForward().normalize() * -1;
	}

	{
		viewport(0, 0, screen_width, screen_height, FAR_CLIP_PLANE, NEAR_CLIP_PLANE);
		Projection = camera.GetProjectionMatrix();
		ModelView = camera.GetModelViewMatrix();
	}
	
	{
		//model->rotate(Vec3f(0, 0, 90));
		//model->ApplyTransform();
	}

	clear_zbuffer();
	TextureShader shader;
	shader.uniform_m =   (Projection);
	shader.uniform_mit = (Projection).invert_transpose();

	Matrix z = ViewPort * Projection * ModelView * model->Transform;


	// Vertex Shader: Should be called per model
	vertex_shader((float*)&z, *(float**)((Vec3f*) &model->verts_), model->nverts(), new_verts);

	#pragma omp parallel for
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		bool out = true;
		#pragma omp parallel for
		for (int j = 0; j < 3; j++) {
			screen_coords[j] = ((Vec4f*)new_verts)[model->faces_[i][j][0]];
			Vec3f screen3(screen_coords[j]);

			shader.varying_uv_coords.set_col(j, model->uv(i, j));
			if (screen3.x > 0 && screen3.x < screen_width && screen3.y > 0 && screen3.y < screen_height) out = false;
		}
		if(!out)
			triangle(screen_coords, shader);
	}
}

