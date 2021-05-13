#include <cmath>

#include "Cone.h"

using namespace std;

bool Cone::intersectLocal(ray& r, isect& i) const
{
	bool ret = false;
	const int x = 0, y = 1, z = 2;	// For the dumb array indexes for the vectors

	glm::dvec3 normal;
	
	glm::dvec3 R0 = r.getPosition();
	glm::dvec3 Rd = r.getDirection();
	double pz = R0[2];
	double dz = Rd[2];
	
	double a = Rd[x]*Rd[x] + Rd[y]*Rd[y] - beta_squared * Rd[z]*Rd[z];

	if( a == 0.0) return false;		// We're in the x-y plane, no intersection

	double b = 2 * (R0[x]*Rd[x] + R0[y]*Rd[y] - beta_squared * ((R0[z] + gamma) * Rd[z]));
	double c = -beta_squared*(gamma + R0[z])*(gamma + R0[z]) + R0[x] * R0[x] + R0[y] * R0[y];

	double discriminant = b * b - 4 * a * c;
	
	double farRoot, nearRoot, theRoot = RAY_EPSILON;
	bool farGood, nearGood;
	
	if(discriminant <= 0) return false;		// No intersection

	discriminant = sqrt(discriminant);

	// We have two roots, so calculate them
	nearRoot = (-b + discriminant) / ( 2 * a );
	farRoot = (-b - discriminant) / ( 2 * a );
	
	// This is confusing, but it figures out which
	// root is closer and puts into theRoot
	nearGood = isGoodRoot(r.at(nearRoot));
	if(nearGood && (nearRoot > theRoot))
	{
		theRoot = nearRoot;
		normal = glm::dvec3((r.at(theRoot))[x], (r.at(theRoot))[y], -2.0 * beta_squared * (r.at(theRoot)[z] + gamma));
	}
	farGood = isGoodRoot(r.at(farRoot));
	if(farGood && ( (nearGood && farRoot < theRoot) || farRoot > RAY_EPSILON) ) 
	{
		theRoot = farRoot;
		normal = glm::dvec3((r.at(theRoot))[x], (r.at(theRoot))[y], -2.0 * beta_squared * (r.at(theRoot)[z] + gamma));
	}

	// In case we are _inside_ the _uncapped_ cone, we need to flip the normal.
	// Essentially, the cone in this case is a double-sided surface
	// and has _2_ normals
	if( !capped && glm::dot(normal, r.getDirection()) > 0 )
		normal = -normal;

	// These are to help with finding caps
	double t1 = (-pz)/dz;
	double t2 = (height-pz)/dz;
	
	glm::dvec3 p( r.at( t1 ) );
	
	if(capped) {
		if( p[0]*p[0] + p[1]*p[1] <=  b_radius*b_radius)
		{
			if(t1 < theRoot && t1 > RAY_EPSILON)
			{
				theRoot = t1;
				if( dz > 0.0 ) {
					// Intersection with cap at z = 0.
					normal = glm::dvec3( 0.0, 0.0, -1.0 );
				} else {
					normal = glm::dvec3( 0.0, 0.0, 1.0 );
				}
			}
		}
		glm::dvec3 q( r.at( t2 ) );
		if( q[0]*q[0] + q[1]*q[1] <=  t_radius*t_radius)
		{
			if(t2 < theRoot && t2 > RAY_EPSILON)
			{
				theRoot = t2;
				if( dz > 0.0 ) {
					// Intersection with interior of cap at z = 1.
					normal = glm::dvec3( 0.0, 0.0, 1.0 );
				} else {
					normal = glm::dvec3( 0.0, 0.0, -1.0 );
				}
			}
		}
	}
	
	if(theRoot <= RAY_EPSILON) return false;
	
	i.setT(theRoot);
	i.setN(glm::normalize(normal));
	i.setObject(this);
	i.setMaterial(this->getMaterial());
	return true;
	
	return ret;
}

bool Cone::isGoodRoot(glm::dvec3 root) const
{

	if(root[2] < 0 || root[2] > height)
		return false;
	return true;
}

