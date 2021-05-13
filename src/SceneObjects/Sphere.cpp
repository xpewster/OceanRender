#include <cmath>

#include "Sphere.h"
#include <iostream>
#include <glm/gtx/io.hpp>

using namespace std;

bool Sphere::intersectLocal(ray& r, isect& i) const
{
	r.setDirection(glm::normalize(r.getDirection()));
	glm::dvec3 v = -r.getPosition();
	double b = glm::dot(v, r.getDirection());
	double discriminant = b*b - glm::dot(v,v) + 1;

	if( discriminant < 0.0 ) {
		return false;
	}

	discriminant = sqrt( discriminant );
	double t2 = b + discriminant;

	if( t2 <= RAY_EPSILON ) {
		return false;
	}

	i.setObject(this);
	i.setMaterial(this->getMaterial());

	double t1 = b - discriminant;

	if( t1 > RAY_EPSILON ) {
		i.setT(t1);
		i.setN(glm::normalize(r.at( t1 )));
	} else {
		i.setT(t2);
		i.setN(glm::normalize(r.at( t2 )));
	}

	return true;
}

