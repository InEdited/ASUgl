#include "util_renderer.h"
#include "util_window.h"

IShader::~IShader() {}

float* z_buffer = new float[screen_width * screen_height];

void viewport(int x, int y, int w, int h, int far_plane, int near_plane) {
	ViewPort[0][3] = x + w / 2.f;
	ViewPort[1][3] = y + h / 2.f;
	ViewPort[2][3] = (far_plane-near_plane) / 2.f;

	ViewPort[0][0] = w / 2.f;
	ViewPort[1][1] = h / 2.f;
	ViewPort[2][2] = (far_plane+near_plane) / 2.f;
}

int color_to_int(TGAColor col) {
	return (col[2] << 16) | (col[1] << 8) | col[0];
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

void triangle( Vec4f* pts, IShader &shader)
{
	Vec3f pts3[3];
	for (int i = 0; i < 3; i++)
		pts3[i] = Vec3f(pts[i]);

	if (pts3[0].y == pts3[1].y && pts3[0].y == pts3[2].y) return; // i dont care about degenerate triangles
	//if (pts3[0].y > pts3[1].y) { std::swap(pts3[0], pts3[1]); }
	//if (pts3[0].y > pts3[2].y) { std::swap(pts3[0], pts3[2]); }
	//if (pts3[1].y > pts3[2].y) { std::swap(pts3[1], pts3[2]); }

	Vec2i bounding_box_min(screen_width - 1, screen_height - 1);
	Vec2i bounding_box_max(0, 0);
	Vec2i clamp(screen_width - 1, screen_height - 1);

	#pragma omp parallel for
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 2; j++) {
			bounding_box_min[j] = std::fmax(0, std::fmin(bounding_box_min[j], (int)pts3[i][j]));
			bounding_box_max[j] = std::fmin(clamp[j], std::fmax(bounding_box_max[j], (int)pts3[i][j]));
		}


	Vec3i P;
	#pragma omp parallel for
	for (P.x = bounding_box_min.x; P.x <= bounding_box_max.x; P.x++) {
		for (P.y = bounding_box_min.y; P.y <= bounding_box_max.y; P.y++) {
			Vec3f bc_coord = barycentric(pts3, P);
			float frag_depth = 0;
			for (int i = 0; i < 3; i++)
				frag_depth += pts3[i][2] * bc_coord[i];
			if (bc_coord.x < 0 || bc_coord.y < 0 || bc_coord.z < 0 || z_buffer[ P.x + P.y * screen_width ]>frag_depth) continue;
			TGAColor color;
			bool discard = shader.fragment(bc_coord, color);
			if (!discard) {
				z_buffer[P.x + P.y * screen_width] = frag_depth;
				set_pixel(P.x, P.y, color_to_int(color));
			}
		}
	}
}