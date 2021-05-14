#ifndef PHOTON_H
#define PHOTON_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "kd_tree.h"

const int photon_resolution = 100;

struct Photon{
    glm::dvec3 location;
    glm::dvec3 direction;
    float strength;

    Photon() = default;
    ~Photon() = default;
    
    Photon(glm::dvec3 location, glm::dvec3 direction, float strength) :
        location(location),
        direction(direction),
        strength(strength) {}
    
    float operator [](std::size_t idx) const {
        return location[idx];
    }
};

class PhotonMap{
public:
    PhotonMap();
    ~PhotonMap();

    void add_photon(Photon photon);
    void update_kd();

    float get_radiance_square(glm::dvec3 center, float w);
    float get_radiance_rect(glm::dvec3 center, float w, float h, float d);

private:
    Kd_tree<Photon>* kd;
    std::vector<Photon> current_set;

};

#endif
