#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/string_cast.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	ray nRay(r);
	
	isect shadowI;
	if (getScene()->intersect (nRay, shadowI)) {
		if (shadowI.getMaterial().Trans()) {
			ray rThrough(nRay);

			rThrough.setPosition(nRay.at(shadowI));// + nRay.at(0.0001));
			//printf ("continuing ray!\n");

			isect transI;
			if (getScene()->intersect (rThrough, transI)) {

				double dist = transI.getT();
				if (dist < RAY_EPSILON) {
					cout << "Im hitting the same point mate" << endl;
					return glm::dvec3(1,1,1);
				}
				glm::dvec3 sha = glm::pow (shadowI.getMaterial().kt(shadowI), glm::dvec3(dist, dist, dist));

				rThrough.setPosition(rThrough.at(transI));
				return sha * shadowAttenuation(rThrough, rThrough.at(transI));
			}
			else {
				//doesn't intersect again for some reason?

				//printf ("WARNING THIS SHOULDN\'T OCCUR\n");
			}
			}
		return glm::dvec3(0,0,0);
	}
	return glm::dvec3(1.0, 1.0, 1.0);
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0

	double d = glm::distance(P, position);

	return 1.0 / (constantTerm + linearTerm * d + quadraticTerm * pow (d, 2.0));
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	ray nRay(r);

	isect shadowI;
	if (getScene()->intersect (nRay, shadowI)) {
		//printf ("%f %f\n", glm::distance(position, p), glm::distance(nRay.at(shadowI.getT()), p));
		glm::dvec3 iPoint = nRay.at(shadowI.getT());
		if (glm::distance(position, p) >= glm::distance(iPoint, p)) {
			
			if (shadowI.getMaterial().Trans()) {
				ray rThrough(nRay);

				rThrough.setPosition(iPoint);// + nRay.at(0.0001));
				//printf ("continuing ray!\n");

				isect transI;
				if (getScene()->intersect (rThrough, transI)) {
					//change later
					double dist = transI.getT();// - shadowI.getT();
					if (dist < RAY_EPSILON) {
						cout << "Im hitting the same point mate" << endl;
						return glm::dvec3(1,1,1);
					}
					glm::dvec3 sha = glm::pow (shadowI.getMaterial().kt(shadowI), glm::dvec3(dist, dist, dist));

					//cout << dist << endl;
					//cout << dist << shadowI.getMaterial().kt(shadowI) << glm::to_string(sha) << endl;

					rThrough.setPosition(rThrough.at(transI));
					return sha * shadowAttenuation(rThrough, rThrough.at(transI));
				}
				else {
					//doesn't intersect again for some reason?

					//printf ("WARNING THIS SHOULDN\'T OCCUR\n");
				}
			}
			//continue ray beyond translucent obj
			//cout << "blocking!" << endl;
			return glm::dvec3(0,0,0);
		}
		//is behind light if here
	}

	return glm::dvec3(1,1,1);
}

#define VERBOSE 0

