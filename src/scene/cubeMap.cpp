#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here
	int index;
	double x = r.getDirection().x;
	double y = r.getDirection().y;
	double z = r.getDirection().z;
	double u,v;
	float absX = fabs(x);
  	float absY = fabs(y);
  	float absZ = fabs(z);
  
  	int isXPositive = x > 0 ? 1 : 0;
  	int isYPositive = y > 0 ? 1 : 0;
  	int isZPositive = z > 0 ? 1 : 0;
  
  	float maxAxis, uc, vc;
  
  	// POSITIVE X
  	if (isXPositive && absX >= absY && absX >= absZ) {
    		// u (0 to 1) goes from +z to -z
    		// v (0 to 1) goes from -y to +y
    		maxAxis = absX;
    		uc = -z;
    		vc = y;
    		index = 0;
  	}
  	// NEGATIVE X
  	if (!isXPositive && absX >= absY && absX >= absZ) {
    		// u (0 to 1) goes from -z to +z
    		// v (0 to 1) goes from -y to +y
    		maxAxis = absX;
    		uc = z;
    		vc = y;
    		index = 1;
 	 }
  	// POSITIVE Y
  	if (isYPositive && absY >= absX && absY >= absZ) {
    		// u (0 to 1) goes from -x to +x
 	   	// v (0 to 1) goes from +z to -z
 	   	maxAxis = absY;
 	   	uc = x;
 	   	vc = -z;
 	   	index = 2;
 	 }
 	 // NEGATIVE Y
 	 if (!isYPositive && absY >= absX && absY >= absZ) {
 	  	 // u (0 to 1) goes from -x to +x
 	   	// v (0 to 1) goes from -z to +z
 	  	 maxAxis = absY;
 	  	 uc = x;
 	 	 vc = z;
 	  	 index = 3;
 	 }
 	 // POSITIVE Z
 	 if (isZPositive && absZ >= absX && absZ >= absY) {
 	  	// u (0 to 1) goes from -x to +x
  	  	// v (0 to 1) goes from -y to +y
  	  	maxAxis = absZ;
  	  	uc = x;
  	  	vc = y;
  	  	index = 4;
  	}
  	// NEGATIVE Z
  	if (!isZPositive && absZ >= absX && absZ >= absY) {
    		// u (0 to 1) goes from +x to -x
    		// v (0 to 1) goes from -y to +y
    		maxAxis = absZ;
    		uc = -x;
    		vc = y;
    		index = 5;
	  }
	
  	// Convert range from -1 to 1 to 0 to 1
  	u = 0.5f * (uc / maxAxis + 1.0f);
  	v = 0.5f * (vc / maxAxis + 1.0f);
	glm::dvec2 scaled (u,v);
	return tMap[index]->getMappedValue(scaled);
}

CubeMap::CubeMap()
{
	for (int i = 0; i < 6; i++) 
		tMap[i] = 0;
}

CubeMap::~CubeMap()
{
}

void CubeMap::setNthMap(int n, TextureMap* m)
{
	if (m != tMap[n].get())
		tMap[n].reset(m);
}
