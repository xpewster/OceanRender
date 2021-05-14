#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here
	glm::dvec3 normals[6] = {glm::dvec3(1, 0, 0),
							glm::dvec3(-1, 0, 0),
							glm::dvec3(0, 1, 0),
							glm::dvec3(0, -1, 0),
							glm::dvec3(0, 0, 1),
							glm::dvec3(0, 0, -1)};
    int min_index = -1;
    double min = 0;
    for (size_t i = 0; i < 6; i++) {
		double t = 1.0 / glm::dot(normals[i], r.getDirection());
        if (t > 0 && (t < min || min_index == -1)) {
            min_index = i;
            min = t;
        }
    }

    glm::dvec3 inter_point = r.getDirection() * min;
    glm::dvec3 proj = inter_point - glm::dot(inter_point, normals[min_index]) * normals[min_index];
    glm::dvec2 proj2(0, 0);

    if (min_index < 2) proj2 = glm::dvec2(proj.z, proj.y);
    if (min_index == 2 || min_index == 3) proj2 = glm::dvec2(proj.x, proj.z);
    if (min_index == 4 || min_index == 5) proj2 = glm::dvec2(proj.x, proj.y);

    glm::dvec2 coord = (proj2+glm::dvec2(1, 1))/2.0;

	return tMap[min_index]->getMappedValue(coord);
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
