#include "wave.h"

const float exponent = 1.0;

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

/*

float Wave::height(glm::vec2 pos, float t){
    return 2.0 * amplitude * pow((sin(glm::dot(direction, pos)*frequency + t*phase_constant)+1.0f)/2.0f, exponent);
}
    
float Wave::height(float x, float y, float t){
    return height(glm::vec2(x,y), t);
}

float Wave::dwdx(glm::vec2 pos, float t){
    return exponent * pow((sin(glm::dot(direction, pos)*frequency + t*phase_constant)+1.0f)/2.0f, exponent-1.0f) 
            * amplitude * frequency * direction.x * cos(glm::dot(direction, pos)*frequency + t*phase_constant);
}

float Wave::dwdz(glm::vec2 pos, float t){
    return exponent * pow((sin(glm::dot(direction, pos)*frequency + t*phase_constant)+1.0f)/2.0f, exponent-1.0f) 
            * amplitude * frequency * direction.y * cos(glm::dot(direction, pos)*frequency + t*phase_constant);
}

*/
