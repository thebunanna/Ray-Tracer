#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

#include <glm/gtx/string_cast.hpp>
#include <iostream>
using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( const auto& pLight : scene->getAllLights() )
	// {
	//              // pLight has type unique_ptr<Light>
	// 		.
	// 		.
	// 		.
	// }

	glm::dvec3 total = glm::dvec3(0,0,0);

	for ( const auto& pLight : scene->getAllLights() )
	{

		glm::dvec3 p = r.at(i.getT());
		glm::dvec3 l = pLight->getDirection(p);
		glm::dvec3 n = i.getN();

		//Let's fire a shadow ray from here to light
		
		glm::dvec3 ref = 2 * glm::dot(n, l) * n - l;

		ray nRay(p, l, glm::dvec3(1,1,1), ray::SHADOW);		

		total += pLight->getColor() * 
			(kd(i) * (max(glm::dot(n, l), 0.0)) +
			ks(i) * pow(max(glm::dot(ref, -r.getDirection()), 0.0), this->shininess(i)) ) *
			min (1.0, pLight->distanceAttenuation(p)) * 
			pLight->shadowAttenuation(nRay, p);
		
		
	}

	return ke(i) + ka(i)*scene->ambient() + total;

	//return kd (i);
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
	//printf ("w: %d, f: %d, d: %ld\n", width, height, data.size());
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.
	double x = coord.x * width;
	double y = coord.y * height;
	
	//When getting corners, decide which pixels should be interp
	//Split current pixel into 4.
	double xpos = trunc(x + 1) - x;
	double ypos = trunc(y + 1) - y;

	glm::dvec3 tl, tr, bl, br;
	int ix = x, iy = y;

	glm::dvec2 tcoords, bias;

	// if (xpos >= 0.5 && ypos >= 0.5) {
	// 	//top left
	// 	tl = getPixelAt(ix - 1, iy - 1);
	// 	tr = getPixelAt(ix, iy - 1);
	// 	bl = getPixelAt(ix - 1, iy);
	// 	br = getPixelAt(ix, iy);
	// 	bias = glm::dvec2(1, 1);
	// }
	// else if (xpos <= 0.5 && ypos >= 0.5) {
	// 	//top right
	// 	tl = getPixelAt(ix, iy - 1);
	// 	tr = getPixelAt(ix + 1, iy - 1);
	// 	bl = getPixelAt(ix, iy);
	// 	br = getPixelAt(ix + 1, iy);
	// 	bias = glm::dvec2(0, 1);
	// }
	// else if (xpos >= 0.5 && ypos <= 0.5) {
	// 	//bottom left
	// 	tl = getPixelAt(ix - 1, iy);
	// 	tr = getPixelAt(ix, iy);
	// 	bl = getPixelAt(ix - 1, iy + 1);
	// 	br = getPixelAt(ix, iy + 1);
	// 	bias = glm::dvec2(1, 0);
	// }
	// else {
	// 	//bottom right
		
	// }
	tl = getPixelAt(ix, iy);
	tr = getPixelAt(ix + 1, iy);
	bl = getPixelAt(ix, iy+1);
	br = getPixelAt(ix + 1, iy + 1);
	bias = glm::dvec2(0, 0);
	//Trunc > ciel since x might be x.0
	double ax = (xpos + bias.x);
	double xb = (1 - ax);
	double cx = (ypos + bias.y);
	double xd = (1 - cx);

	//Get 4 corners of pictures and computer bilinear interp

	///printf ("x: %f - %f, %f, y: %f - %f, %f\n", x, a, b, y, c, d);
	//lecture slides lied to meeeeee
	return xd * (xb * br + ax * bl) + cx * (xb * tr + ax * tl);
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	//since i am looking at 1 + pixels ahead, refit to prevent segfault
	if (x >= width) {
		x = width-1;
	}
	if (y >= height) {
		y = height - 1;
	}

	ulong index = 3 * (y * width + x);
	double r = data[index] / 255.0;
	double g = data[index+1] / 255.0;
	double b = data[index+2] / 255.0;
	//Make sure not to normalize or else colors become really dim.
	return glm::dvec3(r,g,b);
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap)
		return _textureMap->getMappedValue(is.getUVCoordinates());
	else
		return _value;
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
