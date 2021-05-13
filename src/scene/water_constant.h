#ifndef __WCONST_H__
#define __WCONST_H__

#include <glm/glm.hpp>

namespace water {
    const glm::dvec3 water_deep_color = glm::dvec3(0, 0.12, 0.3);
    const int num_geometry_waves = 4;
    const int num_texture_waves = 10;
    const int resolution = 25;
    const float water_size = 1.0f;
    const float sea_level = 0.1f;
    const float seafloor_level = -0.15f;
    const glm::vec3 offset(0.01f , 0.0f, water_size/2.0);
    const glm::vec2 l_bounds(0.01, 0.2);
    const glm::vec2 a_bounds(0.0002, 0.005);
    const glm::vec2 s_bounds(0.5, 3.0);
    const glm::vec2 t_l_bounds(0.01, 0.1);
    const glm::vec2 t_a_bounds(0.001, 0.01);
    const glm::vec3 water_diffuse(0.35, 0.73, 0.85);
    const glm::vec3 water_ambient(0.5, 0.6, 0.8);
    const glm::vec3 water_specular(0.0, 0.0, 0.0);
    const glm::vec3 water_reflectivity(0.0, 0.0, 0.0);
    const glm::vec3 water_transmissivity(1.0, 1.0, 0.5);
    const float water_shininess = 0.25;
}

#endif