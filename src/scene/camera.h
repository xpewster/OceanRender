#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

class Camera
{
public:
    Camera();
    void rayThrough( double x, double y, ray &r );
    void setEye( const glm::dvec3 &eye );
    void setLook( double, double, double, double );
    void setLook( const glm::dvec3 &viewDir, const glm::dvec3 &upDir );
    void setFOV( double );
    void setAspectRatio( double );

    double getAspectRatio() { return aspectRatio; }

	const glm::dvec3& getEye() const			{ return eye; }
	const glm::dvec3& getLook() const		{ return look; }
	const glm::dvec3& getU() const			{ return u; }
	const glm::dvec3& getV() const			{ return v; }
private:
    glm::dmat3 m;                     // rotation matrix
    double normalizedHeight;    // dimensions of image place at unit dist from eye
    double aspectRatio;
    
    void update();              // using the above three values calculate look,u,v
    
    glm::dvec3 eye;
    glm::dvec3 look;                  // direction to look
    glm::dvec3 u,v;                   // u and v in the 
};

#endif
