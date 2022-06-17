#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include "sceneentitybuilder.h"
#include "../res/imagedata.h"

struct MaterialComponent
{
	tex::ImageData m_diffuse;
	tex::ImageData m_bump;
	
	glm::vec4 m_ambient;
	glm::vec4 m_diffuse;
	glm::vec4 m_specular;
	glm::vec4 m_emission;
	float     m_shininess;
};

class MaterialSystem : public ISystem
{
public:
    static MaterialComponent GetDefaultMaterialComponent();

    void        update(Registry & reg, float time_delta) {}
    std::string getName() const { return "MaterialSystem"; }
};

#endif /* MATERIAL_H */
