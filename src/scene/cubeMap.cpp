#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
#include <stdio.h>
#include <iostream>
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
	double x = r.getDirection().x;
	double y = r.getDirection().y;
	double z = r.getDirection().z;

	bool posX = x >= 0;
	bool posY = y >= 0;
	bool posZ = z >= 0;

	double ax = fabs (x), ay = fabs(y), az = fabs (z);

	int index = 0;
	double u, v, scale;

	if (ax >= ay && ax >= az) {
		scale = ax;
		if (posX) {
			u = z;
			v = y;
			index = 0;			
		}
		else {
			u = -z;
			v = y;
			index = 1;
		}
	}
	else if (ay >= ax && ay >= az) {
		scale = ay;
		if (posY) {
			u = x;
			v = z;
			index = 2;
		}
		else {
			u = x;
			v = -z;
			index = 3;
		}
	}
	else if (az >= ay && az >= ax) {
		scale = az;
		if (!posZ) {
			u = x;
			v = y;
			index = 4;
		}
		else {
			u = -x;
			v = y;
			index = 5;
		}
	}
	else {
		printf ("ERROR! Undetected index? \n");
		return glm::dvec3(0,0,0);
	}
	//printf ("%f, %f, %f, %d\n", u, v, scale, index);
	glm::dvec3 color = tMap[index].get()->getMappedValue(glm::dvec2( (u/scale + 1) / 2, (v/scale + 1) / 2));
	return color;
}

CubeMap::CubeMap()
{

}

CubeMap::~CubeMap()
{
	
}

void CubeMap::setNthMap(int n, TextureMap* m)
{
	if (m != tMap[n].get())
		tMap[n].reset(m);
}
