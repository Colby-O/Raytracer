#ifndef SCENE_H_
#define SCENE_H_

#include "DataTypes.h"
#include "Object.h"
#include "Camera.h"
#include "Image.h"
#include "ImagePlane.h"
#include "Ray.h"
#include "PointLight.h"

#include <iostream>
#include <vector>

#define MAX(a,b) (a<b)?b:a

namespace raytracer {
	class Scene;
	struct Objects;
}

class raytracer::Scene {
private:
	Image image_ = Image(500, 500);
	std::vector<Object*> objects_;
	std::vector<LightSource*> lights_;
	Camera camera_ = Camera(Vec3(0.0, 0.0, 0.0));
	ImagePlane imagePlane_ = ImagePlane(image_.getCols(), image_.getRows(), Vec3(-1, -1, -1), Vec3(1, 1, -1));

	uchar clamp(int color) const;
	bool hasShadow(Ray ray, float lightDirLength) const;
	Colour phong(Ray ray, Object* closestObj, Vec3 normal, float t) const;
	Object* findClosestObject(Ray ray, float& t) const;
public:
	Scene() {}
	Scene(std::vector<Object*> objects, std::vector<LightSource*> lights) : objects_(objects), lights_(lights) {}

	void renderScene();
};

uchar raytracer::Scene::clamp(int color) const {
	if (color < 0) return 0;
	if (color >= 255) return 255;
	return color;
}


bool raytracer::Scene::hasShadow(Ray ray, float lightDirLength) const {
	bool hasShadow = false;
	float dist;
	for (int i = 0; i < objects_.size(); ++i) {
		bool hasIntersect = objects_[i]->hasIntersect(ray, dist);
		if (dist > 0.0001 && dist < lightDirLength && hasIntersect) {
			hasShadow = true;
			break;
		}
	}
	return hasShadow;
}

Colour raytracer::Scene::phong(Ray ray, Object* closestObj, Vec3 normal, float t) const {
	Material m = closestObj->getMaterial();
	Vec3 intersection = ray.compute(t);
	Vec3 diffuseAndSpecColour = Vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < lights_.size(); ++i) {
		Vec3 lightVector = lights_[i]->computeLightVector(intersection);
		Vec3 viewer = ray.getOrigin() - intersection;
		float lightDirLength = lightVector.norm();

		lightVector.normalize();
		normal.normalize();
		viewer.normalize();

		float diffuseTerm = 0.0f;
		float specularTerm = 0.0f;
		Ray lightRay = Ray(intersection, lightVector);
		if (!hasShadow(lightRay, lightDirLength)) {
			diffuseTerm = MAX(lightVector.dot(normal), 0);

			Vec3 relfectedLight = 2 * lightVector.dot(normal) * normal - lightVector;
			relfectedLight.normalize();
			specularTerm = pow(MAX(relfectedLight.dot(viewer), 0), m.getSpecularExp());
		}

		Colour diffuseColour = m.getDiffuseColor();
		Colour specularColor = m.getSpecularColor();
		float kd = m.getKd();
		float ks = m.getKs();

		diffuseAndSpecColour[0] += kd * diffuseColour[0] * diffuseTerm + ks * specularColor[0] * specularTerm;
		diffuseAndSpecColour[1] += kd * diffuseColour[1] * diffuseTerm + ks * specularColor[1] * specularTerm;
		diffuseAndSpecColour[2] += kd * diffuseColour[2] * diffuseTerm + ks * specularColor[2] * specularTerm;
	}

	Colour colour(0, 0, 0);
	float ka = m.getKa();
	Colour ambientColour = m.getAmbientColor();
	colour[0] = clamp(ka * ambientColour[0] + diffuseAndSpecColour[0] / (float)lights_.size());
	colour[1] = clamp(ka * ambientColour[1] + diffuseAndSpecColour[1] / (float)lights_.size());
	colour[2] = clamp(ka * ambientColour[2] + diffuseAndSpecColour[2] / (float)lights_.size());
	return colour;
}


raytracer::Object* raytracer::Scene::findClosestObject(Ray ray, float& t) const {
	float dist = 0.0f;
	Object* closestObj = nullptr;

	for (int i = 0; i < objects_.size(); ++i) {
		bool hasIntersect = objects_[i]->hasIntersect(ray, dist);
		if (dist > 0 && dist < t && hasIntersect) {
			closestObj = objects_[i];
			t = dist;
		}
	}

	return closestObj;
}

void raytracer::Scene::renderScene() {

	for (int row = 0; row < image_.getRows(); ++row) {
		for (int col = 0; col < image_.getCols(); ++col) {

			Ray ray = camera_.generateRay(imagePlane_.generatePixelPos(col, row));
			float t = std::numeric_limits<float>::max();
			Object* closestObj = findClosestObject(ray, t);
			if (closestObj != nullptr) {
				Vec3 normal = closestObj->getNormal(ray.compute(t));
				Colour colour = phong(ray, closestObj, normal, t);
				image_(row, col) = colour;
			}
			else {
				image_(row, col) = white();
			}	
			
		}
	}

	image_.save("./result.png");
	image_.display();
}

#endif