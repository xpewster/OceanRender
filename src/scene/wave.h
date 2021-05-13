#ifndef WAVE_H
#define WAVE_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Wave{
public:
    Wave();
    ~Wave();
    Wave(float l, float a, float s, glm::vec2 d);

    float height(glm::vec2 pos, float t);
    float height(float x, float y, float t);
    float dwdx(glm::vec2 pos, float t);
    float dwdz(glm::vec2 pos, float t);
    glm::vec3 normal(glm::vec2 pos, float t);

private:
    float wavelength;
    float frequency;
    float amplitude;
    float speed;
    float phase_constant;
    glm::vec2 direction;
};

#endif