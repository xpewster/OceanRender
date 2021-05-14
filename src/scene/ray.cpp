#include "ray.h"
#include "../ui/TraceUI.h"
#include "material.h"
#include "scene.h"


const Material& isect::getMaterial() const
{
	return material ? *material : obj->getMaterial();
}

ray::ray(const glm::dvec3& pp,
	 const glm::dvec3& dd,
	 const glm::dvec3& w,
         RayType tt, bool uw)
        : p(pp), d(dd), atten(w), t(tt), underwater(uw)
{
	TraceUI::addRay(ray_thread_id);
}



ray::ray(const ray& other) : p(other.p), d(other.d), atten(other.atten)
{
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
	return *this;
}

glm::dvec3 ray::at(const isect& i) const
{
	return at(i.getT());
}

thread_local unsigned int ray_thread_id = 0;
