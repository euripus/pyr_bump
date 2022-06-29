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

    Entity              entity_id;
    Entity              parent;
    std::vector<Entity> children;
};

struct TransformComponent
{
    glm::mat4 new_mat{1.0f};
    bool      replase_local_matrix = false;
};

// tag type
struct IsTransformed
{
    bool val = true;
};

class SceneSystem : public ISystem
{
public:
    static SceneComponent GetDefaultSceneComponent();

    SceneSystem(Registry & reg) : ISystem(reg) {}

    // ISystem interface
    // bool        init() override;
    void        update(float time_delta) override;
    void        postUpdate() override;
    std::string getName() const override { return "Scene positions."; }

    void addNode(Entity node_id, Entity parent = null_entity_id);

    Entity m_root_id;
private:
    bool m_transform_updated = false;

    void updateTransform(Entity ent, bool initiator);
    void updateBound(SceneComponent & node);
    void propagateBoundToRoot(Entity ent);
};
}   // namespace evnt
#endif   // SCENE_H
