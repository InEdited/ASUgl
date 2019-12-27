#include "improv_gfx.h"

void Setup() {

	AddModel(new Model("sakura.obj"));

	AddModel(new Model("african_head.obj"));

	Model* model2 = new Model("african_head.obj");
	model2->translate(Vec3f(1, 0, 0));
	model2->ApplyTransform();
	AddModel(model2);

	Model* model3 = new Model("african_head.obj");
	model3->translate(Vec3f(-1, 0, 0));
	model3->ApplyTransform();
	AddModel(model3);
}
