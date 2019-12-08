#define _USE_MATH_DEFINES
#include <vector>
#include <algorithm>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "renderer.h"
#include "util_window.h"
#include <ctime>

#define DEG2RAD M_PI/180
#define HORIZONTAL_CAMERA_SPEED 1
#define VERTICAL_CAMERA_SPEED 1

#define NEAR_CLIP_PLANE 0 
#define FAR_CLIP_PLANE 15
#define FOV 15

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

const int depth = 255;

int angle_hor = 0;
int angle_ver = 0;

bool wireframe = false;


Model* model = new Model("african_head.obj");

float* z_buffer = new float[screen_width * screen_height];
Vec3f light_dir = Vec3f(0, 0, 1).normalize();
Vec3f eye(0, 0, 5);
Vec3f center(0, 0, 0);

Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity();
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

void line(Vec3f p0, Vec3f p1, TGAColor color)
{
	bool steep = false;

	if (std::abs(p0[0] - p1[0]) < std::abs(p0[1] - p1[1])) {
		std::swap(p0[0], p0[1]);
		std::swap(p1[0], p1[1]);
		steep = true;

	}

	if (p0[0] > p1[0]) {
		std::swap(p0[0], p1[0]);
		std::swap(p0[1], p1[1]);
	}

	int dx = p1[0] - p0[0];
	int dy = p1[1] - p0[1];
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = p0[1];
	int y_step = p1[1] > p0[1] ? 1 : -1;
	int dx_2 = 2 * dx;

	for (int x = p0[0]; x <= p1[0]; x++) {
		if (steep) {
			set_pixel(y, x, color_to_int(color));
		}
		else {
			set_pixel(x, y, color_to_int(color));
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y_step);
			error2 -= dx_2;
		}
	}
}

Vec3f barycentric(Vec3f* pts, Vec3f P)
{
	Vec3f u = cross(
		Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), // AC_x, AB_x, distance_x
		Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1])  // AC_y, AB_y, distance_y
	);

	if (std::abs(u[2]) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}


void triangle(
	Vec3f* pts,         // Needed
	Vec2f* diff_pts,    // Should be removed
	Model* model,
	float* intensities)
{

	if (pts[0].y == pts[1].y && pts[0].y == pts[2].y) return; // i dont care about degenerate triangles
	if (pts[0].y > pts[1].y) {
		std::swap(pts[0], pts[1]);
		if(diff_pts)
			std::swap(diff_pts[0], diff_pts[1]);
		if(intensities)
			std::swap(intensities[0], intensities[1]);
	}
	if (pts[0].y > pts[2].y) {
		std::swap(pts[0], pts[2]);
		if(diff_pts)
			std::swap(diff_pts[0], diff_pts[2]);
		if(intensities)
			std::swap(intensities[0], intensities[2]);
	}
	if (pts[1].y > pts[2].y) {
		std::swap(pts[1], pts[2]);
		if(diff_pts)
			std::swap(diff_pts[1], diff_pts[2]);
		if(intensities)
			std::swap(intensities[1], intensities[2]);
	}

	if (wireframe)
	{
		line(pts[0], pts[1], white);
		line(pts[1], pts[2], white);
		line(pts[2], pts[0], white);
		return;
	}

	Vec2i bounding_box_min(screen_width - 1, screen_height - 1);
	Vec2i bounding_box_max(0, 0);
	Vec2i clamp(screen_width - 1, screen_height - 1);
	TGAColor color = white;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bounding_box_min[j] = std::fmax(0, std::fmin(bounding_box_min[j], (int)pts[i][j]));
			bounding_box_max[j] = std::fmin(clamp[j], std::fmax(bounding_box_max[j], (int)pts[i][j]));
		}
	}

	Vec3f P;
	for (P.x = bounding_box_min.x; P.x <= bounding_box_max.x; P.x++) {
		for (P.y = bounding_box_min.y; P.y <= bounding_box_max.y; P.y++) {
			Vec3f bc_coord = barycentric(pts, P);
			if (bc_coord.x < 0 || bc_coord.y < 0 || bc_coord.z < 0) continue;


			float intensity =
				intensities[0]
				+ (intensities[1] - intensities[0]) * bc_coord[1]
				+ (intensities[2] - intensities[0]) * bc_coord[2];


			// Interpolating Z using the barycentric coordinates
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_coord[i];

			// Coloring according to the Z-Buffer
			if (P.z > z_buffer[(int)(P.x + P.y * screen_width)] && P.z > 20 && P.z < 50)
			{
				z_buffer[(int)(P.x + P.y * screen_width)] = P.z;

				// If diff_pts (Diffusemap Points) were passed, then find the
									 // color of the current pixel
				if (diff_pts) {
					Vec2f diff_pt =
						diff_pts[0]
						+ (diff_pts[1] - diff_pts[0]) * bc_coord[1]
						+ (diff_pts[2] - diff_pts[0]) * bc_coord[2];

					color = model->diffuse(diff_pt);
				}
					color = color * intensity;
				set_pixel(P.x, P.y, color_to_int(color));
				//char debugStr[200];
				//sprintf_s(debugStr, "%f\n", P.z);
				//OutputDebugString(debugStr);
			}
		}
	}
}

int color_to_int(TGAColor col) {
	return (col[2] << 16) | (col[1] << 8) | col[0];
}

void clear_zbuffer()
{
	for (int i = 0; i < screen_width * screen_height; i++)
		z_buffer[i] = INT_MIN;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    Matrix Minv = Matrix::identity();
    Matrix Tr   = Matrix::identity();
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -center[i];
    }
    return Minv*Tr;
}

void move_camera_right() {
	angle_hor += HORIZONTAL_CAMERA_SPEED;
}

void move_camera_left() {
	angle_hor -= HORIZONTAL_CAMERA_SPEED;
}

void move_camera_up() {
	angle_ver += VERTICAL_CAMERA_SPEED;
	if (angle_ver > 90) angle_ver = 90;
}

void move_camera_down() {
	angle_ver -= VERTICAL_CAMERA_SPEED;
	if (angle_ver < -90) angle_ver = -90;
}

Matrix ViewPort = Matrix::identity();
Matrix Projection = Matrix::identity();
Matrix ModelView = Matrix::identity();

void render()
{

	Vec3f forward = Vec3f(
		sinf(angle_hor * DEG2RAD), 
		-sinf(angle_ver * DEG2RAD), 
		-cosf(angle_hor*DEG2RAD) * cosf(angle_ver*DEG2RAD));
	Vec3f right = Vec3f(sinf(angle_hor*DEG2RAD + M_PI_2), 0, 0);

	center = eye + forward;

	//ViewPort = viewport(0, 0, screen_width, screen_height);
	ViewPort = viewport(screen_width / 8, screen_height / 8, screen_width * 3 / 4, screen_height * 3 / 4);
	ModelView = lookat(eye, center, cross(right, forward));
	Projection[0][0] = 1 / tanf(FOV * DEG2RAD);
	Projection[1][1] = 1 / tanf(FOV * DEG2RAD);
	Projection[2][2] = (FAR_CLIP_PLANE + NEAR_CLIP_PLANE) / (FAR_CLIP_PLANE - NEAR_CLIP_PLANE);
	Projection[2][3] = (-2 * FAR_CLIP_PLANE * NEAR_CLIP_PLANE) / (FAR_CLIP_PLANE - NEAR_CLIP_PLANE);
	Projection[3][2] = -1.f / (forward).norm();
	//Projection[3][2] = 1;
	//Projection[3][3] = 0;

//	model->rotate(Vec3f(0, 180, 0));
//	model->ApplyTransform();

	Matrix z = ViewPort * Projection * ModelView * model->Transform;

	clear_zbuffer();
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec2f diffuse_coords[3];
		float intensities[3];
		bool out = true;

		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			Vec4f v4(v);
			Vec3f coord(z * v4);

			if (coord.x > 0 && coord.x < screen_width
				&& coord.y > 0 && coord.y < screen_height)
				out = false;

			screen_coords[j] = coord;
			world_coords[j] = v;
			diffuse_coords[j] = model->uv(i, j);
			intensities[j] = model->normal(i, j) * light_dir;
		}

		if (out) continue;

		triangle(screen_coords, diffuse_coords, model, intensities);
	}
}

