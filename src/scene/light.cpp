#include "light.h"
#include "scene.h"

LightComponent LightSystem::GetDefaultLightComponent(LightType l_type)
{
    LightComponent cmp;

    cmp.type          = l_type;
    cmp.ambient       = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    cmp.diffuse       = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    cmp.specular      = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    cmp.position      = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    cmp.attenuation   = glm::vec3(1.0f, 0.0f, 0.0f);
    cmp.spotDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    cmp.spotExponent  = 0.0f;
    cmp.spotCosCutoff = -1.0f;   // cos 180 градусов == -1

    return cmp;
}

void LightSystem::update(float time_delta)
{
    for(auto ent : m_reg.view<evnt::SceneComponent, LightComponent, evnt::IsTransformed>())
    {
        auto & pos = m_reg.get<evnt::SceneComponent>(ent);
        auto & lgh = m_reg.get<LightComponent>(ent);

        lgh.position = pos.abs * lgh.position;

        if(lgh.type == LightType::Spot)
            lgh.spotDirection = glm::vec3(pos.abs * glm::vec4(lgh.spotDirection, 0.0f));
    }
}
