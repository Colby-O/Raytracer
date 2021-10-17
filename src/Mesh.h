#ifndef MESH_H_
#define MESH_H_

#include <stdio.h>
#include <iostream>
#include "Object.h"
#include "Triangle.h"

namespace raytracer{
	class Mesh;
	struct Face;
}

struct raytracer::Face {
	unsigned int vertexIndex[3];
	unsigned int normalIndex[3];
	unsigned int uvIndex[3];
};

class raytracer::Mesh {
private:
	char* path_ = "bunny.obj";
	std::vector<Vec3> vertices_;
	std::vector<Vec2> uvs_;
	std::vector<Vec3> normals_;
	std::vector<Face> faces_;
	std::vector<Triangle> triangles_;

	Vec3 position_;
	Vec3 rotation_;
	float scale_;

	bool objectFileParser();
public:
	Mesh() {
		objectFileParser();
	}
	Mesh(char* path, Vec3 position, float scale ) : position_(position), scale_(scale){
		path_ = path;
		objectFileParser(); 
	}

	void getObjects(std::vector<Object*> &objects);
};

void raytracer::Mesh::getObjects(std::vector<Object*>& objects) {
	for (int i = 0; i < triangles_.size(); ++i) {
		objects.push_back(&triangles_[i]);
	}
}

bool raytracer::Mesh::objectFileParser() {

	FILE* file = fopen(path_, "r");
	if (file == NULL) {
		std::cout << "Failed to open file."<< std::endl;
		return false;
	}

	while (true) {
		char lineHeader[128];

		int pointer = fscanf(file, "%s", lineHeader);

		if (pointer == EOF) {
			break;
		}
		else if (strcmp(lineHeader, "v") == 0) {
			float vx, vy, vz;
			fscanf(file, "%f %f %f\n", &vx, &vy, &vz);
			vz *= scale_;
			vy *= -scale_;
			vx *= scale_;
			vz += position_[2];
			vy += position_[1];
			vx += position_[0];
			vertices_.push_back(Vec3(vx, vy, vz));
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			float uvx, uvy;
			fscanf(file, "%f %f\n", &uvx, &uvy);
			uvs_.push_back(Vec2(uvx, uvy));
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			float nx, ny, nz;
			fscanf(file, "%f %f %f\n", &nx, &ny, &nz);
			normals_.push_back(Vec3(nx, ny, nz));
		}
		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			Face f;
			char line[128];
			fgets(line, 128, file);
			int matches = sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches == 9) {
				f.vertexIndex[0] = vertexIndex[0];
				f.vertexIndex[1] = vertexIndex[1];
				f.vertexIndex[2] = vertexIndex[2];

				f.normalIndex[0] = normalIndex[0];
				f.normalIndex[1] = normalIndex[1];
				f.normalIndex[2] = normalIndex[2];

				f.uvIndex[0] = uvIndex[0];
				f.uvIndex[1] = uvIndex[1];
				f.uvIndex[2] = uvIndex[2];
			}
			else {
				matches = sscanf(line, "%d//%d %d//%d %d//%d", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches == 6) {
					f.vertexIndex[0] = vertexIndex[0];
					f.vertexIndex[1] = vertexIndex[1];
					f.vertexIndex[2] = vertexIndex[2];

					f.normalIndex[0] = normalIndex[0];
					f.normalIndex[1] = normalIndex[1];
					f.normalIndex[2] = normalIndex[2];
				}
				else {
					matches = sscanf(line, "%d/%d %d/%d %d/%d", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
					if (matches == 6) {
						f.vertexIndex[0] = vertexIndex[0];
						f.vertexIndex[1] = vertexIndex[1];
						f.vertexIndex[2] = vertexIndex[2];

						f.uvIndex[0] = uvIndex[0];
						f.uvIndex[1] = uvIndex[1];
						f.uvIndex[2] = uvIndex[2];
					}
					else {
						unsigned int vertexIndex[3];
						matches = sscanf(line, "%d %d %d", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
						f.vertexIndex[0] = vertexIndex[0];
						f.vertexIndex[1] = vertexIndex[1];
						f.vertexIndex[2] = vertexIndex[2];
					}
				}
			}

			if (matches != 3 && matches != 6 && matches != 9) {
				std::cout << "File can't be read by this parser (Try exporting with other options)" << std::endl;
				return false;
			}
			faces_.push_back(f);
		}
	}

	for (int i = 0; i < faces_.size(); ++i) {
		
		Vec3 v1 = vertices_[faces_[i].vertexIndex[0] - 1];
		Vec3 v2 = vertices_[faces_[i].vertexIndex[1] - 1];
		Vec3 v3 = vertices_[faces_[i].vertexIndex[2] - 1];
		triangles_.push_back(Triangle(v1, v2, v3));
	}
	return true;
}

#endif
