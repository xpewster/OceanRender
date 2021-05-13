#include "trimesh.h"
#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for (auto m : materials)
		delete m;
	for (auto f : faces)
		delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3& v)
{
	vertices.emplace_back(v);
}

void Trimesh::addMaterial(Material* m)
{
	materials.emplace_back(m);
}

void Trimesh::addNormal(const glm::dvec3& n)
{
	normals.emplace_back(n);
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c)
{
	int vcnt = vertices.size();

	if (a >= vcnt || b >= vcnt || c >= vcnt)
		return false;

	TrimeshFace* newFace = new TrimeshFace(
	        scene, new Material(*this->material), this, a, b, c);
	newFace->setTransform(this->transform);
	if (!newFace->degen)
		faces.push_back(newFace);
	else
		delete newFace;

	// Don't add faces to the scene's object list so we can cull by bounding
	// box
	return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char* Trimesh::doubleCheck()
{
	if (!materials.empty() && materials.size() != vertices.size())
		return "Bad Trimesh: Wrong number of materials.";
	if (!normals.empty() && normals.size() != vertices.size())
		return "Bad Trimesh: Wrong number of normals.";

	return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	bool have_one = false;
	for (auto face : faces) {
		isect cur;
		if (face->intersectLocal(r, cur)) {
			if (!have_one || (cur.getT() < i.getT())) {
				i = cur;
				have_one = true;
			}
		}
	}
	if (!have_one)
		i.setT(1000.0);
	return have_one;
}

bool TrimeshFace::intersect(ray& r, isect& i) const
{
	return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
	// YOUR CODE HERE
	//
	// FIXME: Add ray-trimesh intersection

	//Check if ray r intersects the plane
    glm::dvec3 N;
    glm::dvec3 P = r.getPosition();
    glm::dvec3 d = r.getDirection();
    glm::dvec3 v0 = parent->vertices[ids[0]]; //a
    glm::dvec3 v1 = parent->vertices[ids[1]]; //b
    glm::dvec3 v2 = parent->vertices[ids[2]]; //c

    glm::dvec3 p0 = v1;

    N = cross((v1 - v0),(v2 - v0)); 

    glm::dvec3 origin1(0.0, 0.0, 0.0);
    float denom = dot(d, N);
    double t = 0.0;

    if (denom < RAY_EPSILON){
        glm::dvec3 p0l0 = p0 - P;
        t = dot(p0l0, N)/ denom;
        if(t<0) return false;
        //printf("detected intersection\n");
    }
    else return false;

    glm::dvec3 ii = r.at(t); // r.getPosition() + r.getDirection() * t ;
    
    //Check if it intersects the triangle
    //Test each edge

    glm::dvec3 Bary;
    // --- Edge AB ---
    glm::dvec3 AB = v1 - v0; 
    glm::dvec3 AP = ii - v0;
    glm::dvec3 C = cross(AB, AP);
    if (dot(N, C) < 0) return false;
    //printf("detected intersection 1\n");

    //--- BC ---
    glm::dvec3 BC = v2 - v1; 
    glm::dvec3 BP = ii - v1;
    C = cross(BC, BP);
    double u = length(C) / length(N);
    if (dot(N, C) < 0) return false;
    //printf("detected intersection 2\n");

    //--- CA --- .
    glm::dvec3 CA = v0 - v2; 
    glm::dvec3 CP = ii - v2;
    C = cross(CA, CP);
    double v = length(C) / length(N);
    if (dot(N, C) < 0) return false;
    //printf("detected intersection 3\n");

    i.setObject(this);
    
    i.setT(t);
    i.setN(N); //true normal

    //Set barycentric coords
    Bary.x = u;
    Bary.y = v; 
    Bary.z = 1.0 - u - v;
    i.setBary(Bary);
    
    //Set material
    if (!parent->materials.empty()){
        Material m0 = *(parent->materials[ids[0]]);
        Material m1 = *(parent->materials[ids[1]]);
        Material m2 = *(parent->materials[ids[2]]);

        isect m;

        //Interpolates color only for now
        glm::dvec3 amb_color = (Bary.x * m0.ka(m)) + 
                    (Bary.y * m1.ka(m)) + 
                    (Bary.z * m2.ka(m));
        m0.setAmbient(amb_color);

        glm::dvec3 dif_color = (Bary.x * m0.kd(m)) + 
                    (Bary.y * m1.kd(m)) + 
                    (Bary.z * m2.kd(m));
        m0.setDiffuse(dif_color);

        i.setMaterial(m0);
        
    }
    else i.setMaterial(this->getMaterial());

    //Interpolate normal 
    if (traceUI->smShadSw() && !parent->normals.empty()){
        glm::dvec3 phong_n(0.0, 0.0, 0.0);
        glm::dvec3 n0 = parent->normals[ids[0]]; //a
        glm::dvec3 n1 = parent->normals[ids[1]]; //b
        glm::dvec3 n2 = parent->normals[ids[2]]; //c

        phong_n = (Bary.x * n0) + (Bary.y * n1) + (Bary.z * n2);
        i.setN(normalize(phong_n));

    }

    return true;

	// /* Plane intersection */
	// double _dot = glm::dot(normal, r.getDirection());
	// if (_dot == 0)
	// 	return false;

	// glm::dvec3 a = parent->vertices[ids[0]];
	// glm::dvec3 b = parent->vertices[ids[1]];
	// glm::dvec3 c = parent->vertices[ids[2]];

	// //double d = -(normal.x*a.x + normal.y*a.y + normal.z*a.z);
	// double t = (dist-glm::dot(normal, r.getPosition()))/_dot;

	// glm::dvec3 q = r.at(t);

	// double aa = glm::length(glm::cross((c-b), (q-b)))/2.0;
	// double ab = glm::length(glm::cross((a-c), (q-c)))/2.0;
	// double ac = glm::length(glm::cross((b-a), (q-a)))/2.0;
	// double A = glm::length(glm::cross((c-b), (a-b)))/2.0;

	// double alpha = aa/A;
	// double beta = ab/A;
	// double gamma = ac/A;

	// if (alpha >= 0 && beta >= 0 && gamma >= 0 && alpha+beta+gamma > 1.0-RAY_EPSILON && alpha+beta+gamma < 1.0+RAY_EPSILON){
	// 	i.setObject(this);
	// 	i.setT(t);
	// 	i.setUVCoordinates(glm::dvec2(alpha,beta));
	// 	i.setBary(alpha, beta, gamma);
	// 	if (parent->materials.empty()){
	// 		i.setMaterial(this->getMaterial());
	// 	}
	// 	else {
	// 		Material ma = *parent->materials[ids[0]];
	// 		Material mb = *parent->materials[ids[1]];
	// 		Material mc = *parent->materials[ids[2]];
	// 		isect m;

	// 		ma.setAmbient(alpha*ma.ka(m) + beta*mb.ka(m) + gamma*mc.ka(m));
	// 		ma.setDiffuse(alpha*ma.kd(m) + beta*mb.kd(m) + gamma*mc.kd(m));
	// 		i.setMaterial(ma);
	// 	}
		
	// 	if (parent->normals.empty()) {
	// 		i.setN(normal);
	// 	}
	// 	else {
	// 		glm::dvec3 na = parent->normals[ids[0]];
	// 		glm::dvec3 nb = parent->normals[ids[1]];
	// 		glm::dvec3 nc = parent->normals[ids[2]];
			
	// 		i.setN(glm::normalize(alpha*na + beta*nb + gamma*nc));
	// 	}

	// 	return true;
	// }

	// return false;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals()
{
	int cnt = vertices.size();
	normals.resize(cnt);
	std::vector<int> numFaces(cnt, 0);

	for (auto face : faces) {
		glm::dvec3 faceNormal = face->getNormal();

		for (int i = 0; i < 3; ++i) {
			normals[(*face)[i]] += faceNormal;
			++numFaces[(*face)[i]];
		}
	}

	for (int i = 0; i < cnt; ++i) {
		if (numFaces[i])
			normals[i] /= numFaces[i];
	}

	vertNorms = true;
}

