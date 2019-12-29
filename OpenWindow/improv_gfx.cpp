#include "improv_gfx.h"

Model *octopus, *ocean, *sharky;

void Setup() {
	octopus = new Model("octojet.obj", 1);
	octopus->scale(Vec3f(4, 4, 4));
	//octopus->translate(Vec3f(0, -0.15, 0));
	octopus->ApplyTransform();
	AddModel(octopus);
	ocean = new Model("ocean.obj", 1);
	ocean->scale(Vec3f(1, 1, 1));
	ocean->ApplyTransform();
	AddModel(ocean);
	sharky = new Model("sharky.obj", 0);
	sharky->scale(Vec3f(0.02, 0.02, 0.02));
	sharky->translate(Vec3f(0, 0.05, 0));
	sharky->ApplyTransform();
	AddModel(sharky);
}

void Draw() {
	sharky->translate(Vec3f(sharky->Translation[0][3] + 0.01*cos(TIME), sharky->Translation[1][3], sharky->Translation[2][3] + 0.01*sin(TIME)));
	sharky->rotate(Vec3f(0, (cos(TIME) + sin(TIME)) * -90, 0));
	sharky->ApplyTransform();

}
