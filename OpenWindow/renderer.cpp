#include "model.h"
#include "renderer.h"
#include "util_window.h"
#include "camera.h"
#include "util_renderer.h"
#include "colladamodel.h"

#define HORIZONTAL_CAMERA_SPEED           0.1
#define VERTICAL_CAMERA_SPEED             0.1
#define VERTICAL_CAMERA_CLAMP_UP           90
#define VERTICAL_CAMERA_CLAMP_DOWN        -90
#define NEAR_CLIP_PLANE                     0 
#define FAR_CLIP_PLANE                     10
#define FOV                                30
#define CAMERA_MOVEMENT_SPEED           0.05f
#define DEFAULT_CAMERA_POS     Vec3f(0, 0, 5)
#define DEFAULT_CAMERA_ROT     Vec3f(0, 0, 0)
#define LIGHT_INTENSITY                   2.f

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Matrix ViewPort = Matrix::identity();
Matrix ModelView = Matrix::identity();
Matrix Projection = Matrix::identity();

ColladaModel* model = new ColladaModel("sssssssssssss.dae");
Camera camera;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();

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

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv_coords.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vertix(iface, nthvert));
		return ViewPort * Projection * ModelView * gl_Vertex;									// transform it to screen coordinates
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
			color[i] = std::fmin(1 + c[i] * (diff_intensity + 0.8 * spec_intensity), 255) * LIGHT_INTENSITY;
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
	

	//Matrix z = ViewPort * Projection * ModelView * model->Transform;

	clear_zbuffer();
	TextureShader shader;
	shader.uniform_m =   (Projection);
	shader.uniform_mit = (Projection).invert_transpose();

	#pragma omp parallel for
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		bool out = true;
		#pragma omp parallel for
		for (int j = 0; j < 3; j++) {
			screen_coords[j] = shader.vertex(i, j);
			Vec3f screen3(screen_coords[j]);

			if (screen3.x > 0 && screen3.x < screen_width && screen3.y > 0 && screen3.y < screen_height) out = false;
		}
		if(!out)
			triangle(screen_coords, shader);
	}
}

