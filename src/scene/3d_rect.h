#ifndef RECT_H
#define RECT_H

#include "split.h"

#include <glm/glm.hpp>


struct prism{
    glm::vec3 center;
    float w;
    float h;
    float d;
    prism() {}
    prism(glm::vec3 center, float w, float h, float d) :
        center(center), w(w), h(h), d(d) {}
    void set(glm::vec3 center, float w, float h, float d) {
        this->center = center;
        this->w = w;
        this->h = h;
        this->d = d;
    }
    bool contains(glm::vec3 point){
        return (point[0] < center[0]+w/2.0f && point[0] > center[0]-w/2.0f && point[1] < center[1]+h/2.0f && point[1] 
                        > center[1]-h/2.0f && point[2] < center[2]+d/2.0f && point[2] > center[2]-d/2.0f);
    }
    bool contains(prism& other){
        return (other.center[0]+other.w/2.0f < center[0]+w/2.0f && other.center[0]-other.w/2.0f > center[0]-w/2.0f 
                        && other.center[1]+other.h/2.0f < center[1]+h/2.0f && other.center[1]-other.h/2.0f > center[1]-h/2.0f 
                        && other.center[2]+other.d/2.0f < center[2]+d/2.0f && other.center[2]-other.d/2.0f > center[2]-d/2.0f);
    }
    bool overlaps(prism& other){
        return ((center[0]+w/2.0f > other.center[0]-other.w/2.0f) && (center[0]-w/2.0f < other.center[0]+other.w/2.0f)
                    && (center[1]+h/2.0f > other.center[1]-other.h/2.0f) && (center[1]-h/2.0f < other.center[1]+other.h/2.0f)
                    && (center[2]+d/2.0f > other.center[2]-other.d/2.0f) && (center[2]-d/2.0f < other.center[2]+other.d/2.0f));
    }
    float operator [](std::size_t idx) const {
        return *(&w + idx);
    }
    glm::vec3 offset(SPLIT_TYPE split){
        glm::vec3 off(0.0);
        off[split] = (*this)[split]/2.0f;
        return off;
    }
};

#endif