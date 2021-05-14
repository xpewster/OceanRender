#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

#include "water_constant.h"

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
	glm::dvec3 it = ke(i);
	it += ka(i) * scene->ambient();

	for(const auto& pl : scene->getAllLights()){
		glm::dvec3 atten = pl->distanceAttenuation(r.at(i.getT())) * pl->shadowAttenuation(r, r.at(i.getT())+RAY_EPSILON*(i.getN()));
		glm::dvec3 diffuse = kd(i)*max(glm::dot(pl->getDirection(r.at(i.getT())), i.getN()), 0.0);
		glm::dvec3 specular = ks(i)*pow(max(glm::dot(glm::reflect(pl->getDirection(r.at(i.getT())), i.getN()), r.getDirection()), 0.0), shininess(i));
		it += atten*pl->getColor()*(diffuse + specular);
	}
	

	return it;
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
}

double epsilon_floor(double d){
	// double f = floor(d);
	// if ((f+1) - d < RAY_EPSILON)
	// 	return f+1;
	// return f;
	return (d > -0.5 && d < 0) ? 0 : floor(d);
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

	double w = (double) getWidth();
	double h = (double) getHeight();
	double u = coord.x *w -1.0;
	double v = coord.y *h -1.0;

	// double sx = 0.0000001;
	double fu =  epsilon_floor(u) ;
	double cu = fu +1.0 ;
	double fv =  epsilon_floor(v) ;
	double cv =  fv + 1.0 ;




	glm::dvec2 upperLeft (fu,cv);
	glm::dvec2 upperRight (cu,cv);
	glm::dvec2 lowerLeft (fu,fv);
	glm::dvec2 lowerRight (cu,fv);
	glm::dvec3 upperLeftPix, upperRightPix, lowerLeftPix, lowerRightPix;
	upperLeftPix = getPixelAt((int)upperLeft.x,(int)upperLeft.y);
	upperRightPix = getPixelAt((int)upperRight.x,(int)upperRight.y);
	lowerLeftPix = getPixelAt((int)lowerLeft.x,(int)lowerLeft.y);
	lowerRightPix = getPixelAt((int)lowerRight.x,(int)lowerRight.y);

	//Bilinear Interoplation here, for RGB of each corner.
	glm::dvec3 q11r = (upperLeftPix);

	glm::dvec3 q12r = (upperRightPix);

	glm::dvec3 q21r = (lowerLeftPix);

	glm::dvec3 q22r = (lowerRightPix);


	glm::dvec3 colorC (0.0, 0.0, 0.0);

	
	glm::dvec3 R2 = ((upperRight.x - u)/(upperRight.x - upperLeft.x)) * q12r
				+ ((u - upperLeft.x)/(upperRight.x - upperLeft.x)) * q22r;
	glm::dvec3 R1 = ((lowerRight.x - u)/(lowerRight.x - lowerLeft.x)) * q11r 
				+ ((u - lowerLeft.x)/(lowerRight.x - lowerLeft.x)) * q21r;
	glm::dvec3 P = ((upperLeft.y - v)/(upperLeft.y - lowerLeft.y)) * R1 
				+ ((v - lowerLeft.y)/(upperLeft.y - lowerLeft.y)) * R2;
	colorC = P;


	return colorC;
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.

	if (data.size() == 0)
      return glm::dvec3(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return glm::dvec3(double(data[pos]) / 255.0, 
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0);
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
