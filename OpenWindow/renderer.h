#ifndef RENDERER_HEADER
#define RENDERER_HEADER
#include "tgaimage.h"
#include "camera.h"


extern float* z_buffer;
extern Camera camera;
extern float TIME;

void init_camera();
void render();
int color_to_int(TGAColor col);
#endif