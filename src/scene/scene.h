#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <optional>
#include "AABB.h"
#include "sceneentitybuilder.h"

namespace evnt
{
struct SceneComponent
{
    glm::mat4           abs;
    glm::mat4           rel;
    std::optional<AABB> bbox;
    std::string         name;
    bool                is_transformed;

    Entity              entity_id;
    Entity              parent;
    std::vector<Entity> children;
};

struct TransformComponent
{
    glm::mat4 new_mat{1.0f};
    bool      replase_local_matrix = false;

    // Entity node_id = null_entity_id;
};

class SceneSystem : public ISystem
{
public:
    static SceneComponent GetDefaultSceneComponent();

    // ISystem interface
    bool        init() override;
    void        update(Registry & reg, float time_delta) override;
    void        postUpdate() override;
    std::string getName() const override { return "Scene positions."; }

    void addNode(SceneComponent node, Entity parent = null_entity_id);

    Entity   m_root_id;
    Registry m_reg;

private:
    void updateTransform(Entity ent, bool initiator);
    void updateBound(SceneComponent & node);
    void propagateBoundToRoot(Entity ent);
};
}   // namespace evnt
#endif   // SCENE_H
