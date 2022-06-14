#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

enum class LightType { Point, Directional, Spot };

struct LightComponent
{
	LightType    type;
	glm::vec4    ambient;
	glm::vec4    diffuse;
	glm::vec4    specular;
	glm::vec4    position;
	glm::vec3    attenuation;
	glm::vec3    spotDirection;
	float        spotExponent;
	float        spotCosCutoff;
};

#endif /* LIGHT_H */
