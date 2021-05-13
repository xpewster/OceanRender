#ifndef __SQUARE_H__
#define __SQUARE_H__

#include "../scene/scene.h"

class Square
	: public MaterialSceneObject
{
public:
	Square( Scene *scene, Material *mat )
		: MaterialSceneObject( scene, mat )
	{
	}

	virtual bool intersectLocal(ray& r, isect& i ) const;
	virtual bool hasBoundingBoxCapability() const { return true; }

    virtual BoundingBox ComputeLocalBoundingBox()
    {
        BoundingBox localbounds;
        localbounds.setMin(glm::dvec3(-0.5f, -0.5f, -RAY_EPSILON));
		localbounds.setMax(glm::dvec3(0.5f, 0.5f, RAY_EPSILON));
        return localbounds;
    }

protected:
	void glDrawLocal(int quality, bool actualMaterials, bool actualTextures) const;

};

#endif // __SQUARE_H__
