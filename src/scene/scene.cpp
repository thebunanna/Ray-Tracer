#include <cmath>

#include "scene.h"
#include "light.h"
#include "kdTree.h"
#include "../SceneObjects/trimesh.h"
#include "../ui/TraceUI.h"
#include <glm/gtx/extended_min_max.hpp>
#include <iostream>
#include <glm/gtx/io.hpp>

using namespace std;

bool Geometry::intersect(ray& r, isect& i) const {
	double tmin, tmax;
	if (hasBoundingBoxCapability() && !(bounds.intersect(r, tmin, tmax))) return false;
	// Transform the ray into the object's local coordinate space
	glm::dvec3 pos = transform->globalToLocalCoords(r.getPosition());
	glm::dvec3 dir = transform->globalToLocalCoords(r.getPosition() + r.getDirection()) - pos;
	double length = glm::length(dir);
	dir = glm::normalize(dir);
	// Backup World pos/dir, and switch to local pos/dir
	glm::dvec3 Wpos = r.getPosition();
	glm::dvec3 Wdir = r.getDirection();
	r.setPosition(pos);
	r.setDirection(dir);
	bool rtrn = false;
	if (intersectLocal(r, i))
	{
		// Transform the intersection point & normal returned back into global space.
		i.setN(transform->localToGlobalCoordsNormal(i.getN()));
		i.setT(i.getT()/length);
		rtrn = true;
	}
	// Restore World pos/dir
	r.setPosition(Wpos);
	r.setDirection(Wdir);
	return rtrn;
}

bool Geometry::hasBoundingBoxCapability() const {
	// by default, primitives do not have to specify a bounding box.
	// If this method returns true for a primitive, then either the ComputeBoundingBox() or
    // the ComputeLocalBoundingBox() method must be implemented.

	// If no bounding box capability is supported for an object, that object will
	// be checked against every single ray drawn.  This should be avoided whenever possible,
	// but this possibility exists so that new primitives will not have to have bounding
	// boxes implemented for them.
	return false;
}

void Geometry::ComputeBoundingBox() {
    // take the object's local bounding box, transform all 8 points on it,
    // and use those to find a new bounding box.

    BoundingBox localBounds = ComputeLocalBoundingBox();
        
    glm::dvec3 min = localBounds.getMin();
    glm::dvec3 max = localBounds.getMax();

    glm::dvec4 v, newMax, newMin;

    v = transform->localToGlobalCoords( glm::dvec4(min[0], min[1], min[2], 1) );
    newMax = v;
    newMin = v;
    v = transform->localToGlobalCoords( glm::dvec4(max[0], min[1], min[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(min[0], max[1], min[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(max[0], max[1], min[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(min[0], min[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(max[0], min[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(min[0], max[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(max[0], max[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
		
    bounds.setMax(glm::dvec3(newMax));
    bounds.setMin(glm::dvec3(newMin));
}

Scene::Scene()
{
	ambientIntensity = glm::dvec3(0, 0, 0);
}

Scene::~Scene()
{
}

void Scene::add(Geometry* obj) {
	obj->ComputeBoundingBox();
	sceneBounds.merge(obj->getBoundingBox());
	objects.emplace_back(obj);
}

void Scene::add(Light* light)
{
	lights.emplace_back(light);
}


// Get any intersection with an object.  Return information about the 
// intersection through the reference parameter.
bool Scene::intersect(ray& r, isect& i) const {
	// double tmin = 0.0;
	// double tmax = 0.0;
	// bool have_one = false;
	// for(const auto& obj : objects) {
	// 	isect cur;
		
	// 	if( obj->intersect(r, cur) ) {
	// 		if(!have_one || (cur.getT() < i.getT())) {
	// 			i = cur;
	// 			have_one = true;
	// 		}
	// 	}
	// }
	// if(!have_one)
	// 	i.setT(1000.0);

	isect other;
	bool res = bvh->intersect(r, other);

	// if (res && other.getT() >= 1000) {
	// 	printf ("Da fuck did you do mate \n");
	// }
	i = other;
	// if (other.getT() != i.getT()) {
	// 	printf ("ERROR! \n");
	// }
	// if debugging,
	if (TraceUI::m_debug)
	{
		addToIntersectCache(std::make_pair(new ray(r), new isect(i)));
	}
	return res;
}

void Scene::computeBVH () {
	bvh = new BVHNode (this);
	bvh->split();
}

TextureMap* Scene::getTexture(string name) {
	auto itr = textureCache.find(name);
	if (itr == textureCache.end()) {
		textureCache[name].reset(new TextureMap(name));
		return textureCache[name].get();
	}
	return itr->second.get();
}


BVHNode::BVHNode(Scene* s) {
    parent = 0;
    box = s->bounds();
    geom = std::vector<Geometry*>();

    for (auto obj = s->beginObjects(); obj < s->endObjects(); obj ++) {

		Trimesh* t = dynamic_cast<Trimesh*>((*obj).get());
		if (t) {
			auto trigeom = std::vector<Geometry*>();
			for (auto fiter = t->beginFaces(); fiter < t->endFaces(); fiter++) {
				(*fiter)->ComputeBoundingBox();
				trigeom.emplace_back(*fiter);
			}
			BVHNode* bvh = new BVHNode(this, trigeom);
			bvh->split();
			this->children.emplace_back(bvh);
		}
		else {
        	geom.emplace_back ((*obj).get());
		}
    }
}

BVHNode::BVHNode(BVHNode* p, std::vector<Geometry*> g) {
    geom = g;
    parent = p;
    box = BoundingBox();
    //Naivly recompute bounding box
    for (auto x : geom) {
        box.merge (x->getBoundingBox());
    }
}

BVHNode::~BVHNode() {
    for (auto x : children) {
        delete x;
    }
}

bool compx (Geometry* a, Geometry* b) {
	auto ca = a->getBoundingBox().getCenter();
	auto cb = b->getBoundingBox().getCenter();
	return ca.x < cb.x;
}
bool compy (Geometry* a, Geometry* b) {
	auto ca = a->getBoundingBox().getCenter();
	auto cb = b->getBoundingBox().getCenter();
	return ca.y < cb.y;
}
bool compz (Geometry* a, Geometry* b) {
	auto ca = a->getBoundingBox().getCenter();
	auto cb = b->getBoundingBox().getCenter();
	return ca.z < cb.z;
}

void BVHNode::split() {

    if (geom.size() <= 1) {
        return;
    }
	if (geom.size() == 2) {
		//In case of only 2 obj, don't bother and just separate
		for (auto x : geom) {
			vector<Geometry*> v;
			v.emplace_back(x);
			children.emplace_back(new BVHNode (this, v));
			return;
		}
	}

    glm::dvec3 maxi = box.getMax();
    glm::dvec3 mini = box.getMin();
    glm::dvec3 diff = maxi - mini;
	glm::dvec3 sumi = maxi + mini;

	int s = geom.size();

    BoundingBox leftHalf, rightHalf;
    if (diff.x >= diff.y && diff.x >= diff.z) {
        // leftHalf = BoundingBox(mini, glm::dvec3(sumi.x * 0.5f, maxi.y, maxi.z));
        // rightHalf = BoundingBox(glm::dvec3(sumi.x * 0.5f, mini.y, mini.z), maxi);
		std::nth_element(geom.begin(), geom.begin() + s/2, geom.end(), compx);

    }
    else if (diff.y >= diff.x && diff.y >= diff.z) {
        // leftHalf = BoundingBox(mini, glm::dvec3(maxi.x, sumi.y * 0.5f, maxi.z));
        // rightHalf = BoundingBox(glm::dvec3(mini.x, sumi.y * 0.5f, mini.z), maxi);
		std::nth_element(geom.begin(), geom.begin() + s/2, geom.end(), compy);

    }
    else {
        // leftHalf = BoundingBox(mini, glm::dvec3(maxi.x, maxi.y, sumi.z * 0.5f));
        // rightHalf = BoundingBox(glm::dvec3(mini.x, mini.y, sumi.z * 0.5f), maxi);
		std::nth_element(geom.begin(), geom.begin() + s/2, geom.end(), compz);

    }

    std::vector<Geometry*> geomLeft, geomRight, geomBoth;

	for (auto iter = geom.begin(); iter < geom.begin() + s/2; iter++) {
		geomLeft.emplace_back(*iter);
	}
	for (auto iter = geom.begin() + s/2; iter < geom.end(); iter++) {
		geomRight.emplace_back(*iter);
	}
	
	/*

    int cl = 0,cr = 0;
	int count = 0;
    for (auto x : geom) {
		bool l = leftHalf.intersects(x->getBoundingBox());
		bool r = rightHalf.intersects(x->getBoundingBox());
        if (l && r) {

			geomBoth.emplace_back(x);
            count ++;
            
        }
		else if (l) {
			geomLeft.emplace_back(x);
			cl++;
		}
        else {
            geomRight.emplace_back(x);
            cr++;
        }
    }
	*/

	//really really lazy here mate
	// for (auto x : geomBoth) {
	// 	BoundingBox nleft = leftHalf.expand(x->getBoundingBox());
	// 	BoundingBox nright = rightHalf.expand(x->getBoundingBox());
	// 	double vl = nleft.volume() - leftHalf.volume();
	// 	double vr = nright.volume() - rightHalf.volume();
	// 	if (vl <= vr) {
	// 		geomLeft.emplace_back(x);
	// 		leftHalf = nleft;
	// 		cl++;
	// 	}
	// 	else {
	// 		geomRight.emplace_back(x);
	// 		rightHalf = nright;
	// 		cr++;
	// 	}
		
	// }

	
	auto lbvh = new BVHNode (this, geomLeft);
    children.emplace_back(lbvh);
	
	auto rbvh = new BVHNode (this, geomRight);
    children.emplace_back(rbvh);

	//children.emplace_back (new BVHNode (this, geomBoth));
    //printf ("%d %d\n", count, cl, cr);
	//double trash = (children[0]->box.volume() + children[1]->box.volume()) / box.volume();
	//printf ("Trash: %f\n", trash);

    geom.clear();

    for (auto x : children) {
        x->split();
    }
}

bool BVHNode::intersect(ray &r, isect &i) 
{
	double tMin, tMax;
	bool have_one = false;

	if (box.intersects (r.getPosition()) || box.intersect(r, tMin, tMax)) {
		isect cur;
		for (auto x : geom) {
			
			if (x->intersect(r, cur)) {
				if(!have_one || (cur.getT() < i.getT())) {
					i = cur;
					have_one = true;
				}
			}
		}
		for (auto child : children) {
			isect other;
			if (child->intersect(r, other)) {
				if(!have_one || (other.getT() < i.getT())) {
					i = other;
					have_one = true;
				}
			}
			
		}
	}
	else {
		//printf ("Missed! moving on.... \n");
	}

	if(!have_one)
		i.setT(1000.0);

	return have_one;
}