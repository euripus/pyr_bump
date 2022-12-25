#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <list>
#include <vector>
#include <string>
#include <optional>
#include "AABB.h"
#include "sceneentitybuilder.h"
#include "src/scene/frustum.h"

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

    void updateQueues(Frustum const & frustum1, Frustum const * frustum2);

    std::vector<Entity> & getModelsQueue() { return m_models_queue; }
    Entity                getRoot() const { return m_root; }

private:
    bool   m_transform_updated = false;
    Entity m_root              = null_entity_id;   // root node of the scene

    // Queues for culling
    std::vector<Entity> m_models_queue;

    void updateTransform(Entity ent, bool initiator);
    void updateBound(Entity node_id);
    void propagateBoundToRoot(Entity ent);

    void updateQueuesRec(Frustum const & frustum1, Frustum const * frustum2, Entity node_id);
};
}   // namespace evnt
#endif   // SCENE_H
