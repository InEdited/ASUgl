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
#include "ColladaModel.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

const int depth = 255;

float* z_buffer;
Vec3f light_dir = Vec3f(0, 0, 1).normalize();
Vec3f eye(0, 0, 3);
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

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
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
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
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
	Model* model,       // Should be removed
	Vec2f* diff_pts,    // Should be removed
	float* intensities,
	Vec3f camera_pos)   // Not really sure yet
{

	if (pts[0].y == pts[1].y && pts[0].y == pts[2].y) return; // i dont care about degenerate triangles
	if (pts[0].y > pts[1].y) {
		std::swap(pts[0], pts[1]);
		std::swap(diff_pts[0], diff_pts[1]);
		std::swap(intensities[0], intensities[1]);
	}
	if (pts[0].y > pts[2].y) {
		std::swap(pts[0], pts[2]);
		std::swap(diff_pts[0], diff_pts[2]);
		std::swap(intensities[0], intensities[2]);
	}
	if (pts[1].y > pts[2].y) {
		std::swap(pts[1], pts[2]);
		std::swap(diff_pts[1], diff_pts[2]);
		std::swap(intensities[1], intensities[2]);
	}
	      Vec2i bounding_box_min(screen_width - 1, screen_height - 1);
        Vec2i bounding_box_max(0, 0);
        Vec2i clamp(screen_width - 1, screen_height - 1);
        TGAColor color = white;

        for(int i = 0; i < 3; i++) {
                for(int j =0; j < 2; j++) {
                        bounding_box_min[j] = std::fmax(0, std::fmin(bounding_box_min[j], (int)pts[i][j]));
                        bounding_box_max[j] = std::fmin(clamp[j], std::fmax(bounding_box_max[j], (int)pts[i][j]));
                }
        }

        Vec3f P;
        for(P.x = bounding_box_min.x; P.x <= bounding_box_max.x; P.x++) {
                for(P.y = bounding_box_min.y; P.y <= bounding_box_max.y; P.y++) {
                        Vec3f bc_coord = barycentric(pts, P);
                        if(bc_coord.x < 0 || bc_coord.y < 0 || bc_coord.z < 0) continue;


                        float intensity =
                                intensities[0]
                                + (intensities[1] - intensities[0]) * bc_coord[1]
                                + (intensities[2] - intensities[0]) * bc_coord[2];


                        // Interpolating Z using the barycentric coordinates
                        P.z = 0;
                        for(int i = 0; i < 3; i++) P.z += pts[i][2] * bc_coord[i];

                        // Coloring according to the Z-Buffer
                        if (P.z > z_buffer[(int)(P.x + P.y * screen_width)])
                        {
                                z_buffer[(int)(P.x + P.y * screen_width)] = P.z;

                                // If diff_pts (Diffusemap Points) were passed, then find the
								                     // color of the current pixel
                                if(diff_pts) {
                                        Vec2f diff_pt =
                                                diff_pts[0]
                                                + (diff_pts[1] - diff_pts[0]) * bc_coord[1]
                                                + (diff_pts[2] - diff_pts[0]) * bc_coord[2];

                                        color = model->diffuse(diff_pt);
                                }
								color = color * intensity;
                                set_pixel(P.x, P.y, color_to_int(color));
                        }
                }
        }
}

int color_to_int(TGAColor col) {
	return (col[2] << 16) | (col[1] << 8) | col[0];
}

void init_zbuffer()
{
	z_buffer = new float[screen_width*screen_height];
	for (int i = 0; i < screen_width * screen_height; i++)
		z_buffer[i] = INT_MIN;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
	Vec3f z = (eye - center).normalize();
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

void my_triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) {
			image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y
		}
	}
}

void render()
{
	//Model* model = new Model("african_head.obj");
	//Matrix ViewPort = viewport(screen_width / 8, screen_height / 8, screen_width * 3 / 4, screen_height * 3 / 4);
	//Matrix Projection = Matrix::identity();
	//Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));

	//Projection[3][2] = -1.f / (eye - center).norm();

	//Matrix z = ViewPort * Projection * ModelView * model->Transform;

	//init_zbuffer();
	//for (int i = 0; i < model->nfaces(); i++)
	//{
	//	std::vector<int> face = model->face(i);
	//	Vec3f screen_coords[3];
	//	Vec3f world_coords[3];
	//	Vec2f diffuse_coords[3];
	//	float intensities[3];

	//	for (int j = 0; j < 3; j++)
	//	{
	//		Vec3f v = model->vert(face[j]);
	//		Vec4f v4(v);
	//		Vec3f coord(z * v4);

	//		screen_coords[j] = coord;
	//		world_coords[j] = v;
	//		diffuse_coords[j] = model->uv(i, j);
	//		intensities[j] = model->normal(i, j) * light_dir;
	//	}

	//	triangle(screen_coords, model, diffuse_coords, intensities, Vec3f(0, 0, 5));
	//}

	ColladaModel* model = new ColladaModel("african_head.dae");
	TGAImage image(800, 800, TGAImage::RGB);
	Vec3f light_dir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<Vec3i> face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vertex(face[j][0]);
			screen_coords[j] = Vec2i((v.x + 1.) * 800 / 2., (v.y + 1.) * 800 / 2.);
			world_coords[j] = v;
		}
		my_triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, white);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
}