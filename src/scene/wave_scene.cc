#pragma warning (disable: 4786)

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "../parser/Parser.h"
#include "wave_scene.h"
#include "scene.h"
#include "../SceneObjects/Square.h"
#include "material.h"
#include "light.h"
#include "../ui/TraceUI.h"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <time.h>

#include "water_constant.h"

using namespace water;

const glm::vec3 offset(0.01f , 0.0f, water_size/2.0);
Material water_mat = {glm::dvec3(0, 0, 0), water_ambient, water_specular, water_diffuse, water_reflectivity,  water_transmissivity, water_shininess, water_index};


WaveScene::WaveScene() {
    WaveScene(0.0f);
}

WaveScene::WaveScene(float t) {

	srand(time(NULL));
	_t = t;

    scene = new Scene;
    scene->getCamera().setFOV(45.0f);
    scene->getCamera().setEye(offset);

    scene->add(new DirectionalLight(scene, glm::dvec3(-10, -10, 0), glm::dvec3(1.0, 1.0, 0.98)));
    //scene->add(new DirectionalLight(scene, glm::dvec3(-10, 10, 0), glm::dvec3(1.0, 1.0, 0.98)));

	// MaterialParameter ms(new TextureMap("../../../src/images/water.bmp"));
	// water_mat.setSpecular(ms);

	float _l[15] = {0.01, 0.01, 0.015, 0.02, 0.03, 0.04, 0.05, 0.07, 0.075, 0.1, 0.1, 0.1, 0.2, 0.3, 0.4};
	float _a[15] = {0.0015, 0.001, 0.001, 0.0025, 0.001, 0.001, 0.002, 0.002, 0.003, 0.003, 0.001, 0.004, 0.0015, 0.001, 0.004};

    glm::vec2 wind_dir = glm::vec2(glm::linearRand<float>(-1.0f, 1.0f), glm::linearRand<float>(-1.0f, 1.0f));
	for(int i = 0; i < num_geometry_waves; i++){
		float l = abs(glm::linearRand<float>(l_bounds.x, l_bounds.y));
		float a = abs(glm::linearRand<float>(a_bounds.x, a_bounds.y));
		// float l = _l[i];
		// float a = _a[i];
		float s = glm::linearRand<float>(s_bounds.x, s_bounds.y);
		glm::vec2 dir = glm::normalize(wind_dir + glm::vec2(glm::linearRand<float>(-0.1f, 0.1f), glm::linearRand<float>(-0.1f, 0.1f)));
		Wave w(l, a, s, dir);
		waves.push_back(w);
		combined_waves.push_back(w);
	}
	for(int i = 0; i < num_texture_waves; i++){
		float l = glm::linearRand<float>(t_l_bounds.x, t_l_bounds.y);
		float a = glm::linearRand<float>(t_a_bounds.x, t_a_bounds.y);
		float s = glm::linearRand<float>(s_bounds.x*(t_l_bounds.x/l_bounds.x), s_bounds.y*(t_l_bounds.y/t_l_bounds.y));
		glm::vec2 dir = glm::normalize(wind_dir + glm::vec2(glm::linearRand<float>(-0.1f, 0.1f), glm::linearRand<float>(-0.1f, 0.1f)));
		Wave w(l, a, s, dir);
		combined_waves.push_back(w);
	}

    Trimesh* tmesh = new Trimesh(scene, new Material(water_mat), &scene->transformRoot);

    calculateWaveGeometry(tmesh, waves, water_vertices, water_faces, t);
	calculateWaveNormals(tmesh, waves, water_normals, t);
	//tmesh->generateNormals();

    const char* error;

    if ((error = tmesh->doubleCheck()))
          throw ParserException(error);

	tmesh->buildKdTree();
    scene->add(tmesh);

    createFloor();
}

void WaveScene::calculateWaveGeometry(Trimesh* tm, std::vector<Wave>& waves, std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& faces, float t){
	for(float _x = 0; _x < resolution; _x++){
		for(float _z = 0; _z < resolution; _z++){
			float x = -water_size/2.0f + _x*(water_size/(float)(resolution-1));
			float z = -water_size/2.0f + _z*(water_size/(float)(resolution-1));
			double height = sea_level;
			for(Wave w : waves){
				height += w.height(glm::vec2(x, z), t);
			}
			//vertices.push_back(glm::vec4(x, height, z, 1.0));
            tm->addVertex(glm::dvec3(x, height, z));
		}
	}
	int _x1 = 0;
	int _x2 = 1;
	while (_x1 < resolution-1 && _x2 < resolution){
		for(int i = 0; i < resolution-1; i++){
			int a = (_x1*resolution)+i;
			int b = (_x1*resolution)+i+1;
			int c = (_x2*resolution)+i;
			int d = (_x2*resolution)+i+1;
			// faces.push_back(glm::uvec3(a, b, c));
			// faces.push_back(glm::uvec3(d, c, b));
            tm->addFace(a, b, c);
            tm->addFace(d, c, b);
		}
		_x1++;
		_x2++;
	}
}

void WaveScene::calculateWaveNormals(Trimesh* tm, std::vector<Wave>& waves, std::vector<glm::vec4>& normals, float t){
	for(float _x = 0; _x < resolution; _x++){
		for(float _z = 0; _z < resolution; _z++){
			float x = -water_size/2.0f + _x*(water_size/(float)(resolution-1));
			float z = -water_size/2.0f + _z*(water_size/(float)(resolution-1));
			float dx = 0.0;
			float dz = 0.0;
			for(Wave w : waves){
				dx += w.dwdx(glm::vec2(x, z), t);
				dz += w.dwdz(glm::vec2(x, z), t);
			}
			// normals.push_back(glm::vec4(-dx, 1.0, -dz, 1.0));
            tm->addNormal(glm::normalize(glm::dvec3(-dx, 1.0, -dz)));
			//std::cout << glm::to_string(glm::normalize(glm::dvec3(-dx, 1.0, -dz))) << std::endl;
		}
	}
}

void WaveScene::createFloor(){
    
    Material floor_mat = {glm::dvec3(0), glm::dvec3(0.4,0.3,0.2), glm::dvec3(0), glm::dvec3(0.5, 0.5, 0.8), glm::dvec3(0), glm::dvec3(0), 128, 1.0};
    try {
        MaterialParameter tex(new TextureMap("../../../src/images/sand.png"));
        floor_mat.setDiffuse(tex);
    } catch (TextureMapException &xcpt) {
		std::cerr << xcpt.message() << std::endl;
		std::string msg("Error: could not open file: ");
		msg.append("../../../src/images/sand.png");
	}
    Square* square = new Square(scene, new Material(floor_mat));
    TransformNode* floor_transform = (&scene->transformRoot)->createChild(glm::scale(glm::dvec3(water_size, 1.0, water_size)))
                            ->createChild(glm::rotate(-1.57, glm::dvec3(1, 0, 0)))
                            ->createChild(glm::translate(glm::dvec3(0, 0, seafloor_level)));
    square->setTransform(floor_transform);
    scene->add(square);
}

glm::dvec3 WaveScene::normal(glm::vec2 pos){
	float dx = 0.0;
	float dz = 0.0;
	for(Wave w : waves){
		dx += w.dwdx(glm::vec2(pos[0], pos[1]), _t);
		dz += w.dwdz(glm::vec2(pos[0], pos[1]), _t);
	}
	// normals.push_back(glm::vec4(-dx, 1.0, -dz, 1.0));
	return glm::normalize(glm::dvec3(-dx, 1.0, -dz));
}