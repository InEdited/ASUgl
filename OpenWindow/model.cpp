#include <iostream>
#include <fstream>
#include <sstream>
#include "model.h"

#define PI 3.14159265358979323846
#define DEG2RAD PI/180

Model::Model(const char *filename) : verts_(), faces_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_() {
    std::ifstream in;
	Transform = Matrix::identity();
	Rotation = Matrix::identity();
	Scale = Matrix::identity();
	Translation = Matrix::identity();
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i=0;i<3;i++) iss >> n[i];
            norms_.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i=0;i<2;i++) iss >> uv[i];
            uv_.push_back(uv);
        }  else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i=0; i<3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm_tangent.tga",      normalmap_);
    load_texture(filename, "_spec.tga",    specularmap_);
}

Model::~Model() {}

void Model::ApplyTransform() {
	Transform = Translation * Scale * Rotation;
}

void Model::translate(Vec3f tr) {
	Translation[0][3] = tr.x;
	Translation[1][3] = tr.y;
	Translation[2][3] = tr.z;
}
void Model::rotate(Vec3f rot) {
	rot = rot * DEG2RAD;

	Rotation[0][0] = cosf(rot.y) * cosf(rot.z);
	Rotation[0][1] = -cosf(rot.y) * sinf(rot.z);
	Rotation[0][2] = sinf(rot.y);
	Rotation[1][0] = sinf(rot.x)*sinf(rot.y)*cosf(rot.z) + cosf(rot.x) * sinf(rot.z);
	Rotation[1][1] = -sinf(rot.x)*sinf(rot.y)*sinf(rot.z) + cosf(rot.x) * cosf(rot.z);
	Rotation[1][2] = -sinf(rot.x)*cosf(rot.y);
	Rotation[2][0] = -cosf(rot.x)*sinf(rot.y)*cosf(rot.z) + sinf(rot.x) * sinf(rot.z);
	Rotation[2][1] = cosf(rot.x)*sinf(rot.y)*sinf(rot.z) + sinf(rot.x) * cosf(rot.z);
	Rotation[2][2] = cosf(rot.x)*cosf(rot.y);
}
void Model::scale(Vec3f scl) {
	Scale[0][0] = scl.x;
	Scale[1][1] = scl.y;
	Scale[2][2] = scl.z;
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    std::vector<int> face;
    for (int i=0; i<(int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
    return face;
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert][0]];
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2f uvf) {
    Vec2i uv(uvf[0]*diffusemap_.get_width(), uvf[1]*diffusemap_.get_height());
    return diffusemap_.get(uv[0], uv[1]);
}

Vec3f Model::normal(Vec2f uvf) {
    Vec2i uv(uvf[0]*normalmap_.get_width(), uvf[1]*normalmap_.get_height());
    TGAColor c = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (int i=0; i<3; i++)
        res[2-i] = (float)c[i]/255.f*2.f - 1.f;
    return res;
}

Vec2f Model::uv(int iface, int nthvert) {
    return uv_[faces_[iface][nthvert][1]];
}

float Model::specular(Vec2f uvf) {
    Vec2i uv(uvf[0]*specularmap_.get_width(), uvf[1]*specularmap_.get_height());
    return specularmap_.get(uv[0], uv[1])[0]/1.f;
}

Vec3f Model::normal(int iface, int nthvert) {
    int idx = faces_[iface][nthvert][2];
    return norms_[idx].normalize();
}

