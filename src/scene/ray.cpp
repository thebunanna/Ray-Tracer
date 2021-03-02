#include "ray.h"
#include "../ui/TraceUI.h"
#include "material.h"
#include "scene.h"
#include <stdio.h>

const Material& isect::getMaterial() const
{
	return material ? *material : obj->getMaterial();
}

void isect::interpolateBary (std::vector<glm::dvec3> normals, int a, int b, int c) 
{

	glm::dvec3 nNorm(this->bary[0] * normals[a] + 
	this->bary[1] * normals[b] +
	this->bary[2] * normals[c]);
	//std::cout << this->bary << std::endl;
	setN(glm::normalize(nNorm));
}

void isect::interpolateMaterial (std::vector<Material*> mats, int a, int b, int c) 
{
	Material newMat;
	newMat += this->bary[0] * *mats[a];
	newMat += this->bary[1] * *mats[b];
	newMat += this->bary[2] * *mats[c];
	setMaterial (newMat);
}

ray::ray(const glm::dvec3& pp,
	 const glm::dvec3& dd,
	 const glm::dvec3& w,
         RayType tt)
        : p(pp), d(dd), atten(w), t(tt)
{
	index = 1.0;
	TraceUI::addRay(ray_thread_id);
}

ray::ray(const ray& other) : p(other.p), d(other.d), atten(other.atten)
{
	index = 1.0;
	TraceUI::addRay(ray_thread_id);
}

ray::~ray()
{
}

ray& ray::operator=(const ray& other)
{
	p     = other.p;
	d     = other.d;
	atten = other.atten;
	t     = other.t;
	index = other.index;
	return *this;
}

glm::dvec3 ray::at(const isect& i) const
{
	return at(i.getT());
}

bool ray::check (const SceneObject* obj) const
{
	if (obj == blacklist) {
		printf ("encountered BL\n");
		return true;
	}
	return false;
}

thread_local unsigned int ray_thread_id = 0;
