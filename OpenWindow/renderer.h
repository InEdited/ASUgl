#ifndef RENDERER_HEADER
#define RENDERER_HEADER
#include "tgaimage.h"


extern int angle_hor;
extern int angle_ver;
extern float* z_buffer;

void move_camera_right();
void move_camera_left();
void move_camera_up();
void move_camera_down();
void render();
int color_to_int(TGAColor col);
#endif