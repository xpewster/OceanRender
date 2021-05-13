#include "camera.h"
#include "../ui/TraceUI.h"

#define PI 3.14159265359
#define SHOW(x) (cerr << #x << " = " << (x) << "\n")

using namespace std;

Camera::Camera()
{
    aspectRatio = 1;
    normalizedHeight = 1;
    
    eye = glm::dvec3(0,0,0);
    u = glm::dvec3( 1,0,0 );
    v = glm::dvec3( 0,1,0 );
    look = glm::dvec3( 0,0,-1 );
    m = glm::dmat3x3(1.0);
}

void
Camera::rayThrough(double x, double y, ray &r)
// Ray through normalized window point x,y.  In normalized coordinates
// the camera's x and y vary both vary from 0 to 1.
{
	x -= 0.5;
	y -= 0.5;
	glm::dvec3 dir = glm::normalize(look + x * u + y * v);
	r.setPosition(eye);
	r.setDirection(dir);
}

void
Camera::setEye(const glm::dvec3 &eye)
{
    this->eye = eye;
}

void
Camera::setLook(double r, double i, double j, double k)
// Set the direction for the camera to look using a quaternion.  The
// default camera looks down the neg z axis with the pos y axis as up.
// We derive the new look direction by rotating the camera by the
// quaternion rijk.
{
    // FIXME: use GLM instead
    // set look matrix
    m[0][0] = 1.0 - 2.0 * (i * i + j * j);
    m[0][1] = 2.0 * (r * i - j * k);
    m[0][2] = 2.0 * (j * r + i * k);
    
    m[1][0] = 2.0 * (r * i + j * k);
    m[1][1]= 1.0 - 2.0 * (j * j + r * r);
    m[1][2] = 2.0 * (i * j - r * k);
    
    m[2][0] = 2.0 * (j * r - i * k);
    m[2][1] = 2.0 * (i * j + r * k);
    m[2][2] = 1.0 - 2.0 * (i * i + r * r);

    m = glm::transpose(m);
    update();
}

void
Camera::setLook(const glm::dvec3 &viewDir, const glm::dvec3 &upDir)
{
    glm::dvec3 z = -viewDir;          // this is where the z axis should end up
    const glm::dvec3 &y = upDir;      // where the y axis should end up
    glm::dvec3 x = glm::cross(y, z);             // lah,

    //m = Mat3d( x[0],x[1],x[2],y[0],y[1],y[2],z[0],z[1],z[2] ).transpose();
    m = glm::dmat3x3(x, y, z); // Do we need to transpose?

    update();
}

void
Camera::setFOV(double fov)
// fov - field of view (height) in degrees    
{
    fov /= (180.0 / PI);      // convert to radians
    normalizedHeight = 2 * tan(fov / 2);
    update();
}

void
Camera::setAspectRatio(double ar)
// ar - ratio of width to height
{
    aspectRatio = ar;
    update();
}

void
Camera::update()
{
    u = m * glm::dvec3(1, 0, 0) * normalizedHeight*aspectRatio;
    v = m * glm::dvec3(0, 1, 0) * normalizedHeight;
    look = m * glm::dvec3(0, 0, -1);
}
