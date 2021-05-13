//
// scene.h
//
// The Scene class and the geometric types that it can contain.
//

#pragma warning(disable : 4786)

#ifndef __SCENE_H__
#define __SCENE_H__

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include "bbox.h"
#include "camera.h"
#include "material.h"
#include "ray.h"
#include "wave.h"

#include <glm/geometric.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using std::unique_ptr;

class Light;
class Scene;

template <typename Obj>
class KdTree;

class SceneElement {
public:
	virtual ~SceneElement() {}

	Scene* getScene() const { return scene; }

	// For debugging purposes, draws using OpenGL
	virtual void glDraw(int quality, bool actualMaterials,
	                    bool actualTextures) const
	{
	}

protected:
	SceneElement(Scene* s) : scene(s) {}

	Scene* scene;
};

inline glm::dvec3 operator*(const glm::dmat4x4& mat, const glm::dvec3& vec)
{
	glm::dvec4 vec4(vec[0], vec[1], vec[2], 1.0);
	auto ret = mat * vec4;
	return glm::dvec3(ret[0], ret[1], ret[2]);
}

class TransformNode {
protected:
	// information about this node's transformation
	glm::dmat4x4 xform;
	glm::dmat4x4 inverse;
	glm::dmat3x3 normi;

	// information about parent & children
	TransformNode* parent;
	std::vector<TransformNode*> children;

public:
	typedef std::vector<TransformNode*>::iterator child_iter;
	typedef std::vector<TransformNode*>::const_iterator child_citer;

	~TransformNode()
	{
		for (auto c : children)
			delete c;
	}

	TransformNode* createChild(const glm::dmat4x4& xform)
	{
		TransformNode* child = new TransformNode(this, xform);
		children.push_back(child);
		return child;
	}

	// Coordinate-Space transformation
	glm::dvec3 globalToLocalCoords(const glm::dvec3& v)
	{
		return inverse * v;
	}

	glm::dvec3 localToGlobalCoords(const glm::dvec3& v)
	{
		return xform * v;
	}

	glm::dvec4 localToGlobalCoords(const glm::dvec4& v)
	{
		return xform * v;
	}

	glm::dvec3 localToGlobalCoordsNormal(const glm::dvec3& v)
	{
		return glm::normalize(normi * v);
	}

	const glm::dmat4x4& transform() const { return xform; }

protected:
	// protected so that users can't directly construct one of these...
	// force them to use the createChild() method.  Note that they CAN
	// directly create a TransformRoot object.
	TransformNode(TransformNode* parent, const glm::dmat4x4& xform)
	        : children()
	{
		this->parent = parent;
		if (parent == NULL)
			this->xform = xform;
		else
			this->xform = parent->xform * xform;
		inverse = glm::inverse(this->xform);
		normi = glm::transpose(glm::inverse(glm::dmat3x3(this->xform)));
	}
};

class TransformRoot : public TransformNode {
public:
	TransformRoot() : TransformNode(NULL, glm::dmat4x4(1.0)) {}
};

// A Geometry object is anything that has extent in three dimensions.
// It may not be an actual visible scene object.  For example, hierarchical
// spatial subdivision could be expressed in terms of Geometry instances.
class Geometry : public SceneElement {
protected:
	// intersections performed in the object's local coordinate space
	// do not call directly - this should only be called by intersect()
	virtual bool intersectLocal(ray& r, isect& i) const = 0;

public:
	// intersections performed in the global coordinate space.
	bool intersect(ray& r, isect& i) const;

	virtual bool hasBoundingBoxCapability() const;
	const BoundingBox& getBoundingBox() const { return bounds; }
	glm::dvec3 getNormal() { return glm::dvec3(1.0, 0.0, 0.0); }

	virtual void ComputeBoundingBox();

	// default method for ComputeLocalBoundingBox returns a bogus bounding
	// box;
	// this should be overridden if hasBoundingBoxCapability() is true.
	virtual BoundingBox ComputeLocalBoundingBox() { return BoundingBox(); }

	void setTransform(TransformNode* transform)
	{
		this->transform = transform;
	};

	Geometry(Scene* scene) : SceneElement(scene) {}

	// For debugging purposes, draws using OpenGL
	void glDraw(int quality, bool actualMaterials,
	            bool actualTextures) const;

	// The defult does nothing; this is here because it is not required
	// that you implement this function if you create your own scene
	// objects.
	virtual void glDrawLocal(int quality, bool actualMaterials,
	                         bool actualTextures) const
	{
	}

protected:
	BoundingBox bounds;
	TransformNode* transform;
};

// A SceneObject is a real actual thing that we want to model in the
// world.  It has extent (its Geometry heritage) and surface properties
// (its material binding).  The decision of how to store that material
// is left up to the subclass.
class SceneObject : public Geometry {
public:
	virtual const Material& getMaterial() const = 0;
	virtual void setMaterial(Material* m) = 0;

	void glDraw(int quality, bool actualMaterials,
	            bool actualTextures) const;

protected:
	SceneObject(Scene* scene) : Geometry(scene) {}
};

// A simple extension of SceneObject that adds an instance of Material
// for simple material bindings.
class MaterialSceneObject : public SceneObject {
public:
	virtual ~MaterialSceneObject() {}

	virtual const Material& getMaterial() const { return *material; }
	virtual void setMaterial(Material* m) { material.reset(m); }

protected:
	MaterialSceneObject(Scene* scene, Material* mat)
	        : SceneObject(scene), material(mat)
	{
	}

	unique_ptr<Material> material;
};

class Scene {
public:
	typedef std::vector<Light*>::iterator liter;
	typedef std::vector<Light*>::const_iterator cliter;
	typedef std::vector<Geometry*>::iterator giter;
	typedef std::vector<Geometry*>::const_iterator cgiter;

	TransformRoot transformRoot;

	Scene();
	virtual ~Scene();

	void add(Geometry* obj);
	void add(Light* light);

	bool intersect(ray& r, isect& i) const;

	auto beginLights() const { return lights.begin(); }
	auto endLights() const { return lights.end(); }
	const auto& getAllLights() const { return lights; }

	auto beginObjects() const { return objects.cbegin(); }
	auto endObjects() const { return objects.cend(); }

	const Camera& getCamera() const { return camera; }
	Camera& getCamera() { return camera; }

	// For efficiency reasons, we'll store texture maps in a cache
	// in the Scene.  This makes sure they get deleted when the scene
	// is destroyed.
	TextureMap* getTexture(string name);

	// These two functions are for handling ambient light; in the Phong
	// model,
	// the "ambient" light is considered a property of the _scene_ as a
	// whole
	// and hence should be set here.
	glm::dvec3 ambient() const { return ambientIntensity; }
	void addAmbient(const glm::dvec3& ambient)
	{
		ambientIntensity += ambient;
	}

	void glDraw(int quality, bool actualMaterials,
	            bool actualTextures) const;

	const BoundingBox& bounds() const { return sceneBounds; }


private:
	std::vector<std::unique_ptr<Geometry>> objects;
	std::vector<std::unique_ptr<Light>> lights;
	Camera camera;

	// This is the total amount of ambient light in the scene
	// (used as the I_a in the Phong shading model)
	glm::dvec3 ambientIntensity;

	typedef std::map<std::string, std::unique_ptr<TextureMap>> tmap;
	tmap textureCache;

	// Each object in the scene, provided that it has
	// hasBoundingBoxCapability(),
	// must fall within this bounding box.  Objects that don't have
	// hasBoundingBoxCapability()
	// are exempt from this requirement.
	BoundingBox sceneBounds;

	KdTree<Geometry>* kdtree;

	mutable std::mutex intersectionCacheMutex;

public:
	// This is used for debugging purposes only.
	void addToIntersectCache(std::pair<ray*, isect*> isect) const
	{
		intersectionCacheMutex.lock();
		intersectCache.push_back(isect);
		intersectionCacheMutex.unlock();
	}

	void clearIntersectCache() const
	{
		intersectionCacheMutex.lock();
		intersectCache.clear();
		intersectionCacheMutex.unlock();
	}

	mutable std::vector<std::pair<ray*, isect*>> intersectCache;
};

#endif // __SCENE_H__
