#ifndef __WCONST_H__
#define __WCONST_H__

#include <glm/glm.hpp>
#include "material.h"

namespace water {
    const glm::dvec3 water_deep_color = glm::dvec3(0, 0.12, 0.3);
    const int num_geometry_waves = 14;
    const int num_texture_waves = 1;
    const int resolution = 100;
    const float water_size = 1.0f;
    const float sea_level = 0.1f;
    const float seafloor_level = -0.15f;
    const glm::vec2 l_bounds(0.01, 0.4);
    const glm::vec2 a_bounds(0.002, 0.001);
    const glm::vec2 s_bounds(0.5, 3.0);
    const glm::vec2 t_l_bounds(0.0001, 0.001);
    const glm::vec2 t_a_bounds(0.0001, 0.0002);
    const glm::dvec3 water_diffuse(0.35, 0.73, 0.85);
    const glm::dvec3 water_ambient(0.5, 0.6, 0.8);
    const glm::dvec3 water_specular(0.0, 0.0, 0.0);
    const glm::dvec3 water_reflectivity(0.0, 0.0, 0.0);
    const glm::dvec3 water_transmissivity(0.5, 0.5, 0.4);
    const double water_shininess = 0.25;
    const double water_index = 1.333;
}

#endif