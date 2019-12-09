#pragma once
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix ViewPort;
extern Matrix Projection;

void viewport(int x, int y, int w, int h, int far, int near);

struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

//void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);

//void triangle( Vec3f* pts, Vec2f* diff_pts, Model* model, float* intensities)

void triangle(Vec4f* pts, IShader &shader);
