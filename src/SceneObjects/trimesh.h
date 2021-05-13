#ifndef TRIMESH_H__
#define TRIMESH_H__

#include <list>
#include <memory>
#include <vector>

#include "../scene/kdTree.h"
#include "../scene/material.h"
#include "../scene/ray.h"
#include "../scene/scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

class TrimeshFace;

class Trimesh : public MaterialSceneObject {
	friend class TrimeshFace;
	typedef std::vector<glm::dvec3> Normals;
	typedef std::vector<glm::dvec3> Vertices;
	typedef std::vector<TrimeshFace *> Faces;
	typedef std::vector<Material *> Materials;

	Vertices vertices;
	Faces faces;
	Normals normals;
	Materials materials;
	BoundingBox localBounds;

public:
	Trimesh(Scene *scene, Material *mat, TransformNode *transform)
	        : MaterialSceneObject(scene, mat),
	          displayListWithMaterials(0),
	          displayListWithoutMaterials(0)
	{
		this->transform = transform;
		vertNorms = false;
	}

	bool vertNorms;

	bool intersectLocal(ray &r, isect &i) const;

	~Trimesh();

	// must add vertices, normals, and materials IN ORDER
	void addVertex(const glm::dvec3 &);
	void addMaterial(Material *m);
	void addNormal(const glm::dvec3 &);
	bool addFace(int a, int b, int c);

	const char *doubleCheck();

	void generateNormals();

	bool hasBoundingBoxCapability() const { return true; }

	BoundingBox ComputeLocalBoundingBox()
	{
		BoundingBox localbounds;
		if (vertices.size() == 0)
			return localbounds;
		localbounds.setMax(vertices[0]);
		localbounds.setMin(vertices[0]);
		Vertices::const_iterator viter;
		for (viter = vertices.begin(); viter != vertices.end();
		     ++viter) {
			localbounds.setMax(
			        glm::max(localbounds.getMax(), *viter));
			localbounds.setMin(
			        glm::min(localbounds.getMin(), *viter));
		}
		localBounds = localbounds;
		return localbounds;
	}

protected:
	void glDrawLocal(int quality, bool actualMaterials,
	                 bool actualTextures) const;
	mutable int displayListWithMaterials;
	mutable int displayListWithoutMaterials;
};

class TrimeshFace : public MaterialSceneObject {
	Trimesh *parent;
	int ids[3];
	glm::dvec3 normal;
	double dist;

public:
	TrimeshFace(Scene *scene, Material *mat, Trimesh *parent, int a, int b,
	            int c)
	        : MaterialSceneObject(scene, mat)
	{
		this->parent = parent;
		ids[0]       = a;
		ids[1]       = b;
		ids[2]       = c;

		// Compute the face normal here, not on the fly
		glm::dvec3 a_coords = parent->vertices[a];
		glm::dvec3 b_coords = parent->vertices[b];
		glm::dvec3 c_coords = parent->vertices[c];

		glm::dvec3 vab = (b_coords - a_coords);
		glm::dvec3 vac = (c_coords - a_coords);
		glm::dvec3 vcb = (b_coords - c_coords);

		if (glm::length(vab) == 0.0 || glm::length(vac) == 0.0 ||
		    glm::length(vcb) == 0.0)
			degen = true;
		else {
			degen  = false;
			normal = glm::cross(b_coords - a_coords,
			                    c_coords - a_coords);
			normal = glm::normalize(normal);
			dist   = glm::dot(normal, a_coords);
		}
		localbounds = ComputeLocalBoundingBox();
		bounds      = localbounds;
	}

	BoundingBox localbounds;
	bool degen;

	int operator[](int i) const { return ids[i]; }

	glm::dvec3 getNormal() { return normal; }

	bool intersect(ray &r, isect &i) const;
	bool intersectLocal(ray &r, isect &i) const;

	bool hasBoundingBoxCapability() const { return true; }

	BoundingBox ComputeLocalBoundingBox()
	{
		BoundingBox localbounds;
		localbounds.setMax(glm::max(parent->vertices[ids[0]],
		                            parent->vertices[ids[1]]));
		localbounds.setMin(glm::min(parent->vertices[ids[0]],
		                            parent->vertices[ids[1]]));

		localbounds.setMax(glm::max(parent->vertices[ids[2]],
		                            localbounds.getMax()));
		localbounds.setMin(glm::min(parent->vertices[ids[2]],
		                            localbounds.getMin()));
		return localbounds;
	}

	const BoundingBox &getBoundingBox() const { return localbounds; }
};

#endif // TRIMESH_H__
