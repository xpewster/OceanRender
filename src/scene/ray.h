//
// ray.h
//
// The low-level classes used by ray tracing: the ray and isect classes.
//

#ifndef __RAY_H__
#define __RAY_H__

// who the hell cares if my identifiers are longer than 255 characters:
#pragma warning(disable : 4786)

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include "material.h"

class SceneObject;
class isect;

/*
 * ray_thread_id: a thread local variable for statistical purpose.
 */
extern thread_local unsigned int ray_thread_id;

// A ray has a position where the ray starts, and a direction (which should
// always be normalized!)

class ray {
public:
	enum RayType { VISIBILITY, REFLECTION, REFRACTION, SHADOW };

	ray(const glm::dvec3& pp, const glm::dvec3& dd, const glm::dvec3& w,
	    RayType tt = VISIBILITY, bool uw = true);
	ray(const ray& other);
	~ray();

	ray& operator=(const ray& other);

	glm::dvec3 at(double t) const { return p + (t * d); }
	glm::dvec3 at(const isect& i) const;

	glm::dvec3 getPosition() const { return p; }
	glm::dvec3 getDirection() const { return d; }
	glm::dvec3 getAtten() const { return atten; }
	RayType type() const { return t; }
	bool isUnderwater() const { return underwater; }

	void setPosition(const glm::dvec3& pp) { p = pp; }
	void setDirection(const glm::dvec3& dd) { d = dd; }
	void setUnderwater(bool uw) { underwater = uw; }

private:
	glm::dvec3 p;
	glm::dvec3 d;
	glm::dvec3 atten;
	RayType t;
	bool underwater;
	float uw_t;
};


// The description of an intersection point.

class isect {
public:
	isect() : obj(NULL), t(0.0), N(), material(nullptr) {}
	isect(const isect& other)
	{
		copyFromOther(other);
	}

	~isect() { }

	isect& operator=(const isect& other)
	{
		copyFromOther(other);
		return *this;
	}

	void setObject(const SceneObject* o) { obj = o; }

	// Get/Set Time of flight
	void setT(double tt) { t = tt; }
	double getT() const { return t; }
	// Get/Set surface normal at this intersection.
	void setN(const glm::dvec3& n) { N = n; }
	glm::dvec3 getN() const { return N; }

	void setMaterial(const Material& m)
	{
		if (material)
			*material = m;
		else
			material.reset(new Material(m));
	}
	void setUVCoordinates(const glm::dvec2& coords)
	{
		uvCoordinates = coords;
	}
	glm::dvec2 getUVCoordinates() const { return uvCoordinates; }
	void setBary(const glm::dvec3& weights) { bary = weights; }
	void setBary(const double alpha, const double beta, const double gamma)
	{
		setBary(glm::dvec3(alpha, beta, gamma));
	}
	const Material& getMaterial() const;

private:
	void copyFromOther(const isect& other)
	{
		if (this == &other)
			return ;
		obj           = other.obj;
		t             = other.t;
		N             = other.N;
		bary          = other.bary;
		uvCoordinates = other.uvCoordinates;
		if (other.material) {
			setMaterial(*other.material);
		} else {
			material.reset();
		}
	}

	const SceneObject* obj;
	double t;
	glm::dvec3 N;
	glm::dvec2 uvCoordinates;
	glm::dvec3 bary;

	// if this intersection has its own material
	// (as opposed to one in its associated object)
	// as in the case where the material was interpolated
	std::unique_ptr<Material> material;
};

const double RAY_EPSILON = 0.00000001;

#endif // __RAY_H__
