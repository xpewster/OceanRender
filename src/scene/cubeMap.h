#pragma once

#include <memory>
#include <glm/vec3.hpp>

class TextureMap;
class ray;

class CubeMap {
	std::unique_ptr<TextureMap> tMap[6];
public:
	CubeMap();
	~CubeMap();

	void setXposMap(TextureMap* m) {
		setNthMap(0, m);
	}
	void setXnegMap(TextureMap* m) {
		setNthMap(1, m);
	}
	void setYposMap(TextureMap* m) {
		setNthMap(2, m);
	}
	void setYnegMap(TextureMap* m) {
		setNthMap(3, m);
	}
	void setZposMap(TextureMap* m) {
		setNthMap(4, m);
	}
	void setZnegMap(TextureMap* m) {
		setNthMap(5, m);
	}

	void setNthMap(int n, TextureMap* m);

	glm::dvec3 getColor(ray r) const;

};
