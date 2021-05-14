#ifndef __WCONST_H__
#define __WCONST_H__

#include <glm/glm.hpp>
#include "material.h"

namespace water {
    const glm::dvec3 water_deep_color = glm::dvec3(0.01, 0.60, 0.73);
    const int num_geometry_waves = 12;
    const int num_sgeometry_waves = 5;
    const int num_texture_waves = 0;
    const int resolution = 100;
    const float water_size = 1.0f;
    const float sea_level = 0.1f;
    const float seafloor_level = -0.15f;
    const glm::vec2 l_bounds(0.03, 0.3);
    const glm::vec2 a_bounds(0.0002, 0.003);
    const glm::vec2 s_bounds(0.5, 3.0);
    const glm::vec2 s_l_bounds(0.001, 0.03);
    const glm::vec2 s_a_bounds(0.0001, 0.0007);
    const glm::vec2 t_l_bounds(0.001, 0.01);
    const glm::vec2 t_a_bounds(0.001, 0.002);
    const glm::dvec3 water_diffuse(0.21, 0.77, 0.85);
    const glm::dvec3 water_ambient(0.5, 0.6, 0.8);
    const glm::dvec3 water_specular(0.0, 0.0, 0.0);
    const glm::dvec3 water_reflectivity(0.0, 0.0, 0.0);
    const glm::dvec3 water_transmissivity(0.5, 0.5, 0.4);
    const double water_shininess = 0.25;
    const double water_index = 1.333;
    const double water_variance = 0.4f;
}

#endif