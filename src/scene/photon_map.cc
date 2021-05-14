#include "photon_map.h"
#include "water_constant.h"
#include <iostream>
#include <cmath>

const float cone_filter_k = 4.0f;

PhotonMap::PhotonMap(){
    
}

PhotonMap::~PhotonMap(){
    
}

void PhotonMap::add_photon(Photon photon){
    current_set.push_back(photon);
}

void PhotonMap::update_kd(){
    std::cout << current_set.size() << " photons" << std::endl;
    kd = new Kd_tree<Photon>(current_set, water::water_size*2.0);
    current_set.clear();
    //kd->print_tree();
}

float PhotonMap::get_radiance_square(glm::dvec3 center, float w){
    
    std::vector<cone_filter_data<Photon>> result;
    kd->find_square(result, center, w);

    float total = 0.0f;
    for(size_t i = 0; i < result.size(); i++){
        if (result[i].distance < w){
            total += (*result[i].data).strength * (1.0f/(photon_resolution*photon_resolution)) 
                    * std::max(0.0f, 1.0f-result[i].distance/(cone_filter_k*w/2.0f));
        }
    }

    return total;
}

float PhotonMap::get_radiance_rect(glm::dvec3 center, float w, float h, float d){
    
    std::vector<cone_filter_data<Photon>> result;
    kd->find_rect(result, center, w, h, d);

    float total = 0.0f;
    for(size_t i = 0; i < result.size(); i++){
        total += (*result[i].data).strength * (1.0f/(photon_resolution*photon_resolution)) 
                    * std::max(0.0f, 1.0f-result[i].distance/((cone_filter_k*0.5f)*w/2.0f));
    }

    return total;
}