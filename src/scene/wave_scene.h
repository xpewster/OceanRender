
#ifndef __WSCENE_H__
#define __WSCENE_H__

#include "wave.h"
#include "../SceneObjects/trimesh.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/random.hpp>



class WaveScene {
public:
    WaveScene();
    ~WaveScene() {}

    WaveScene(float t);

    Scene* getScene() {return scene;}

private:
    /* Water */
	std::vector<glm::vec4> water_vertices;
	std::vector<glm::vec4> water_normals;
	std::vector<glm::uvec3> water_faces;
	std::vector<Wave> waves;
	std::vector<Wave> combined_waves; //geom and tex waves

    Scene* scene;

    void calculateWaveGeometry(Trimesh* tm, std::vector<Wave>& waves, std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& faces, float t);
    void calculateWaveNormals(Trimesh* tm, std::vector<Wave>& waves, std::vector<glm::vec4>& normals, float t);

    void createFloor();
};

#endif