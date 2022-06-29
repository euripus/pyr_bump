#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include "sceneentitybuilder.h"
#include "../res/imagedata.h"

struct MaterialComponent
{
    tex::ImageData m_diff;
    tex::ImageData m_bump;

    uint32_t m_base_tex_id;
    uint32_t m_diff_tex_id;

    glm::vec4 m_ambient;
    glm::vec4 m_diffuse;
    glm::vec4 m_specular;
    glm::vec4 m_emission;
    float     m_shininess;
};

class MaterialSystem
{
public:
    static MaterialComponent GetDefaultMaterialComponent();

    // void        update(float time_delta) override {}
    // std::string getName() const override { return "MaterialSystem"; }

    static bool LoadTGA(MaterialComponent & mat, std::string const & base_fname,
                        std::string const & bump_fname);
};

#endif /* MATERIAL_H */
