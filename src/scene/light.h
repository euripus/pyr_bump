#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "sceneentitybuilder.h"

enum class LightType
{
    Point,
    Directional,
    Spot
};

struct LightComponent
{
    LightType type;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 position;
    glm::vec3 attenuation;
    glm::vec3 spotDirection;
    float     spotExponent;
    float     spotCosCutoff;
};

class LightSystem : public ISystem
{
public:
    static LightComponent GetDefaultLightComponent(LightType l_type = LightType::Point);

    void        update(Registry & reg, float time_delta);
    std::string getName() const { return "LightSystem"; }
};

#endif /* LIGHT_H */
