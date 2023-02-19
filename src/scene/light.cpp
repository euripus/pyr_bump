#include "light.h"
#include "scenecmp.h"

LightComponent LightSystem::GetDefaultLightComponent(LightType l_type)
{
    LightComponent cmp;

    cmp.type            = l_type;
    cmp.ambient         = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    cmp.diffuse         = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    cmp.specular        = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    cmp.position        = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    cmp.attenuation     = glm::vec3(1.0f, 0.0f, 0.0f);
    cmp.spot_direction  = glm::vec3(0.0f, 0.0f, -1.0f);
    cmp.spot_exponent   = 0.0f;
    cmp.spot_cos_cutoff = -1.0f;   // cos 180 градусов == -1

    return cmp;
}

void LightSystem::update(double time)
{
    for(auto ent : m_reg.view<SceneComponent, LightComponent, Event::Scene::IsTransformed>())
    {
        auto const & pos = m_reg.get<SceneComponent>(ent);
        auto &       lgh = m_reg.get<LightComponent>(ent);

        if(lgh.type == LightType::Spot || lgh.type == LightType::Point)
        {
            lgh.position       = pos.abs * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            lgh.spot_direction = glm::mat3(pos.abs) * lgh.spot_direction;
        }

        if(lgh.type == LightType::Directional)
            lgh.spot_direction = glm::vec3(pos.abs * glm::vec4(lgh.spot_direction, 0.0f));
    }
}
