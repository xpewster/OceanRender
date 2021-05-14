#include "photon_map.h"
#include "water_constant.h"
#include <iostream>

PhotonMap::PhotonMap(){
    
}

PhotonMap::~PhotonMap(){
    
}

void PhotonMap::add_photon(Photon photon){
    current_set.push_back(photon);
}

void PhotonMap::update_kd(){
    std::cout << current_set.size() << std::endl;
    kd = new Kd_tree<Photon>(current_set, water::water_size*2.0);
    current_set.clear();
    //kd->print_tree();
}

float PhotonMap::get_radiance_square(glm::dvec3 center, float w){
    std::vector<Photon*> result;
    kd->find_square(result, center, w);

    float total = 0.0f;
    for(size_t i = 0; i < result.size(); i++){
        total += (*result[i]).strength * 0.01f;
    }

    return total;
}