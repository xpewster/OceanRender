#include "wave.h"

Wave::Wave(){
    
}

Wave::~Wave(){

}

Wave::Wave(float l, float a, float s, glm::vec2 d){
    wavelength = l;
    frequency = 2*glm::pi<float>()/l;
    amplitude = a;
    speed = s;
    phase_constant = s*frequency;
    direction = d;
}

float Wave::height(glm::vec2 pos, float t){
    return amplitude * sin(glm::dot(direction, pos)*frequency + t*phase_constant);
}
    
float Wave::height(float x, float y, float t){
    return height(glm::vec2(x,y), t);
}

float Wave::dwdx(glm::vec2 pos, float t){
    return amplitude * frequency * direction.x * cos(glm::dot(direction, pos)*frequency + t*phase_constant);
}

float Wave::dwdz(glm::vec2 pos, float t){
    return amplitude * frequency * direction.y * cos(glm::dot(direction, pos)*frequency + t*phase_constant);
}

glm::vec3 Wave::normal(glm::vec2 pos, float t){
    return glm::normalize(glm::vec3(-dwdx(pos, t), 1.0, -dwdz(pos, t)));
}