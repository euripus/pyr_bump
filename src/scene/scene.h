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

struct SceneComponent
{
    glm::mat4                 abs;
    glm::mat4                 rel;
    std::optional<evnt::AABB> initial_bbox;
    std::optional<evnt::AABB> transformed_bbox;
    std::string               name;

    Entity            entity_id;
    Entity            parent;
    std::list<Entity> children;
};

namespace Event
{
namespace Scene
{
    struct TransformComponent
    {
        glm::mat4 new_mat{1.0f};
        bool      replase_local_matrix = false;
    };

    struct IsTransformed
    {};

    struct IsBboxUpdated
    {};
}   // namespace Scene
}   // namespace Event

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

    void connectNode(Entity node_id, Entity parent = null_entity_id);
    void disconnectNode(Entity node_id);

    void updateQueues(evnt::Frustum const & frustum1, evnt::Frustum const * frustum2);

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

    void updateQueuesRec(evnt::Frustum const & frustum1, evnt::Frustum const * frustum2, Entity node_id);
};

#endif   // SCENE_H
