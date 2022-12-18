#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <list>
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
    std::optional<AABB> initial_bbox;
    std::optional<AABB> transformed_bbox;
    std::string         name;

    Entity            entity_id;
    Entity            parent;
    std::list<Entity> children;
};

struct TransformComponent
{
    glm::mat4 new_mat{1.0f};
    bool      replase_local_matrix = false;
};

// tag type
struct IsTransformed
{};

struct IsBboxUpdated
{};

class SceneSystem : public ISystem
{
public:
    static SceneComponent GetDefaultSceneComponent();

    SceneSystem(Registry & reg) : ISystem(reg) {}

    // ISystem interface
    // bool        init() override;
    void        update(double time) override;
    void        postUpdate() override;
    std::string getName() const override { return "Scene positions."; }

    void addNode(Entity node_id, Entity parent = null_entity_id);

private:
    bool m_transform_updated = false;

    void updateTransform(Entity ent, bool initiator);
    void updateBound(Entity node_id);
    void propagateBoundToRoot(Entity ent);
};
}   // namespace evnt
#endif   // SCENE_H
