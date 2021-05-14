#pragma once

#ifndef __KD__
#define __KD__

#include <algorithm>
#include <vector>
#include <cmath>
#include "ray.h"
#include "bbox.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;


template <typename Obj>
struct splitPlane {
	double position;
	int leftCount;
	int rightCount;
	BoundingBox leftBbox;
	BoundingBox rightBbox;
	Obj* face;
	bool isRight;
};

template <typename Obj>
bool comparePlanes(splitPlane<Obj>*a, splitPlane<Obj>* b) {
	return a->position < b->position;
}

template <typename Obj>
class LeafNode;

template <typename Obj>
class SplitNode;

template <typename Obj>
class KdTree {

protected:
	enum NodeType {
		SPLIT,
		LEAF
	};

public:
	NodeType nodetype;

	KdTree(){}
	virtual ~KdTree(){}

	virtual bool findIntersection(double tmin, double tmax, ray& r, isect& i) = 0;

	KdTree<Obj>* buildTree(std::vector<Obj*>& objList, BoundingBox bbox, int depth) {
		unsigned int leafsizebound = traceUI->getLeafSize();
		int depthlimit = traceUI->getMaxDepth();
		if (objList.size() <= leafsizebound || ++depth == depthlimit)
			return new LeafNode<Obj>(objList);

		double bBoxArea = bbox.area();

		std::vector<Obj*> leftList, rightList;

		double minSam = 1.0e100;
		int bestAxis = 0;
		splitPlane<Obj>* minPlane;
		splitPlane<Obj> bestPlane;
		typedef std::vector<splitPlane<Obj>*> SplitPlaneList;
		SplitPlaneList candidateList;
		splitPlane<Obj>* newPlane;

		for (int axis = 0; axis < 3; axis++) {
			// make list of candidate planes
			for (auto& o : objList) {
				newPlane = new splitPlane<Obj>();
				newPlane->position = o->getBoundingBox().getMin()[axis];
				newPlane->face = o;
				newPlane->isRight = false;
				candidateList.push_back(newPlane);
				newPlane = new splitPlane<Obj>();
				newPlane->position = o->getBoundingBox().getMax()[axis];
				newPlane->face = o;
				newPlane->isRight = true;
				candidateList.push_back(newPlane);
			}

			if (candidateList.size() == 0) throw "Empty candidate list\n";

			// sort candidate list
			std::sort(candidateList.begin(), candidateList.end(), comparePlanes<Obj>);

			// sweep to get object counts and bounding boxes
			splitPlane<Obj> sweep;
			sweep.leftBbox.setEmpty();
			sweep.rightBbox.setEmpty();
			sweep.position = bbox.getMin()[axis];
			sweep.leftCount = sweep.rightCount = 0;

			typedef typename std::vector<splitPlane<Obj>*>::const_iterator splIter;
			// left to right sweep for left bboxes and counts
			for (splIter pl = candidateList.begin(); pl != candidateList.end(); ++pl) {
				splitPlane<Obj>* plane = *pl;
				if (plane->position > sweep.position  || (pl + 1) == candidateList.end()) {
					sweep.leftBbox.merge(sweep.rightBbox);
					sweep.leftCount += sweep.rightCount;
					sweep.rightCount = 0;
				}
				if (!plane->isRight) {
					sweep.rightBbox.merge(plane->face->getBoundingBox());
					sweep.rightCount++;
				}
				plane->leftCount = sweep.leftCount;
				plane->leftBbox = sweep.leftBbox;
				sweep.position = plane->position;
			}

			sweep.leftBbox.setEmpty();
			sweep.rightBbox.setEmpty();
			sweep.position = bbox.getMax()[axis];
			sweep.leftCount = sweep.rightCount = 0;

			typedef typename std::vector<splitPlane<Obj>*>::const_reverse_iterator splrIter;
			// right to left sweep for right bboxes and counts
			for (splrIter pl = candidateList.rbegin(); pl != candidateList.rend(); ++pl) {
				splitPlane<Obj>* plane = *pl;
				if (plane->position < sweep.position || (pl + 1) == candidateList.rend()) {
					sweep.rightBbox.merge(sweep.leftBbox);
					sweep.rightCount += sweep.leftCount;
					sweep.leftCount = 0;
				}
				if (plane->isRight) {
					sweep.leftBbox.merge(plane->face->getBoundingBox());
					sweep.leftCount++;
				}
				plane->rightCount = sweep.rightCount;
				plane->rightBbox = sweep.rightBbox;
				sweep.position = plane->position;
			}

			// get minimum SAM cost split plane
			minPlane = &bestPlane;
			for (splIter pl = candidateList.begin(); pl != candidateList.end(); ++pl) {
				splitPlane<Obj>* plane = *pl;
				if (plane->position < plane->leftBbox.getMax()[axis]) plane->leftBbox.setMax(axis, plane->position);
				if (plane->position > plane->rightBbox.getMin()[axis]) plane->rightBbox.setMin(axis, plane->position);
				double sam = (plane->leftCount * plane->leftBbox.area() + plane->rightCount * plane->rightBbox.area()) / bBoxArea;
				if (sam < minSam) {
					minSam = sam;
					bestAxis = axis;
					minPlane = plane;
				}
			}
			bestPlane = *minPlane;
			candidateList.clear();
		}

		double position = bestPlane.position;
		//split the object list at chosen plane
		for (const auto& obj : objList) {
			if (obj->getBoundingBox().getMin()[bestAxis] < position)
				leftList.emplace_back(obj);
			if (obj->getBoundingBox().getMax()[bestAxis] > position)
				rightList.emplace_back(obj);
			if (obj->getBoundingBox().getMin()[bestAxis] == position &&
			    obj->getBoundingBox().getMax()[bestAxis] == position) {
				if (obj->getNormal()[bestAxis] < 0.0)
					leftList.emplace_back(obj);
				else
					rightList.emplace_back(obj);
			}
		}

		// recur
		if (leftList.size() == 0 || rightList.size() == 0 ) return new LeafNode<Obj>(objList);
		return new SplitNode<Obj>(bestAxis, position,
					  buildTree(leftList,
						    bestPlane.leftBbox,
						    depth),
					  buildTree(rightList,
						    bestPlane.rightBbox,
						    depth));
	}
};

template <typename Obj>
class SplitNode : public KdTree<Obj> {

	const int axis;
	const double position;

	KdTree<Obj>* left;
	KdTree<Obj>* right;

public:
 SplitNode(const int ax, const double pos) : axis(ax), position(pos) { this->nodetype = KdTree<Obj>::SPLIT; }
	SplitNode(const int ax, const double pos, KdTree<Obj>* leftChild, KdTree<Obj>* rightChild)
		: axis(ax), position(pos) 
	{ 
	  this->nodetype = KdTree<Obj>::SPLIT;
		addLeft(leftChild);
		addRight(rightChild);
	}
	~SplitNode() { }

	bool addLeft(KdTree<Obj>* treenode) {
		left = treenode;
		return true;
	}
	bool addRight(KdTree<Obj>* treenode) {
		right = treenode;
		return true;
	}
	KdTree<Obj>* Left() { return left; }
	KdTree<Obj>* Right() { return right; }

	bool findIntersection(double tMin, double tMax, ray& r, isect& i) {
		double rP = r.getPosition()[axis];
		double rD = r.getDirection()[axis];
		if (rD > -RAY_EPSILON && rD < RAY_EPSILON) {
			if (rP < position - RAY_EPSILON) {
				if (left->findIntersection(tMin, tMax, r, i)) return true;
				else return false;
			}
			if (rP > position + RAY_EPSILON) {
				if (right->findIntersection(tMin, tMax, r, i)) return true;
				else return false;
			}
			bool tempA = left->findIntersection(tMin, tMax, r, i);
			isect tempI = i;
			bool tempB = right->findIntersection(tMin, tMax, r, i);
			if (tempI.getT() < i.getT()) i = tempI;
			return (tempA || tempB);
		} else {
			double tInt = (position - rP)/rD;
			if (rD > 0.0) {
				if (tInt > tMax + RAY_EPSILON) {
					if (left->findIntersection(tMin, tMax, r, i)) return true;
				} else if (tInt < tMin - RAY_EPSILON) {
					if (right->findIntersection(tMin, tMax, r, i)) return true;
				} else {
					if (left->findIntersection(tMin, tInt, r, i)) return true;
					if (right->findIntersection(tInt, tMax, r, i)) return true;
				}
				return false;
			} else if (rD < 0.0) {
				if (tInt > tMax + RAY_EPSILON) {
					if (right->findIntersection(tMin, tMax, r, i)) return true;
				}
				else if (tInt < tMin - RAY_EPSILON) {
					if (left->findIntersection(tMin, tMax, r, i)) return true;
				}
				else {
					if (right->findIntersection(tMin, tInt, r, i)) return true;
					if (left->findIntersection(tInt, tMax, r, i)) return true;
				}
				return false;
			}
		}
		return false;
	}

};

template <typename Obj>
class LeafNode : public KdTree<Obj> {
  
  std::vector<Obj*> objList;

public:
	LeafNode() { this->nodetype = KdTree<Obj>::LEAF; }
	LeafNode(const std::vector<Obj*>& olist)
	{
		this->nodetype = KdTree<Obj>::LEAF;
		objList = olist;
	}
	~LeafNode() {}

	void addObject(Obj* o)
	{
		objList.push_back(o);
	}

	bool findIntersection(double tMin, double tMax, ray& r, isect& i)
	{
		bool hit = false;
		for (auto obj : objList) {
			isect cur;
			if(obj->intersect(r, cur) && cur.getT() >= tMin - RAY_EPSILON && cur.getT() <= tMax + RAY_EPSILON)
			{
				if(!hit || cur.getT() < i.getT() )
				{
					i = cur;
					hit = true;
				}
			}
		}
		if(!hit) i.setT(1000.0);
		return hit;
	}
};
#endif 
