#include "scene.h"

evnt::SceneComponent evnt::SceneSystem::GetDefaultSceneComponent()
{
    evnt::SceneComponent node;
    // set defaults
    node.abs            = glm::mat4(1.0f);
    node.rel            = glm::mat4(1.0f);
    node.entity_id      = null_entity_id;
    node.parent         = null_entity_id;
    node.is_transformed = false;

    return node;
}

void evnt::SceneSystem::update(Registry & reg, float time_delta)
{
    bool in_loop = false;

    for(auto ent : reg.view<SceneComponent, TransformComponent>())
    {
        if(!in_loop)
        {
            in_loop             = true;
            m_transform_updated = true;
        }

        auto & trans = reg.get<TransformComponent>(ent);
        auto & pos   = reg.get<SceneComponent>(ent);

        if(trans.replase_local_matrix)
            pos.rel = trans.new_mat;
        else
            pos.rel *= trans.new_mat;

        updateTransform(ent, true);
    }

    // clear all TransformComponent
    if(m_transform_updated)
        reg.reset<TransformComponent>();
}

void evnt::SceneSystem::postUpdate()
{
    if(m_transform_updated)
    {
        for(auto ent : m_reg.view<SceneComponent>())
        {
            auto & pos = m_reg.get<SceneComponent>(ent);

            pos.is_transformed = false;
        }
        m_transform_updated = false;
    }
}

void evnt::SceneSystem::addNode(Entity node_id, Entity parent)
{
    auto & node = m_reg.get<SceneComponent>(node_id);

    if(parent == null_entity_id)
    {
        m_root_id = node_id;
        updateBound(node);
    }
    else
    {
        auto & parent_node = m_reg.get<SceneComponent>(parent);
        node.parent        = parent;

        parent_node.children.push_back(node_id);
        updateTransform(node_id, true);

        m_transform_updated = true;
    }
}

void evnt::SceneSystem::updateTransform(Entity ent, bool initiator)
{
    auto & node = m_reg.get<SceneComponent>(ent);

    node.is_transformed = true;

    if(NotNull(node.parent))
    {
        auto & parent_node = m_reg.get<SceneComponent>(node.parent);
        node.abs           = parent_node.abs * node.rel;   //!!!!
    }
    else
    {
        node.abs = node.rel;
    }

    for(auto & ch : node.children)
    {
        updateTransform(ch, false);
    }

    updateBound(node);

    if(initiator && NotNull(node.parent))
        propagateBoundToRoot(node.parent);
}

void evnt::SceneSystem::updateBound(SceneComponent & node)
{
    if(node.bbox)
        node.bbox->transform(node.abs);

    for(auto & ch : node.children)
    {
        auto & child_node = m_reg.get<SceneComponent>(ch);

        if(child_node.bbox)
        {
            if(!node.bbox)
                node.bbox = AABB{};

            node.bbox->expandBy(*child_node.bbox);
        }
    }
}

void evnt::SceneSystem::propagateBoundToRoot(Entity ent)
{
    auto & node = m_reg.get<SceneComponent>(ent);

    updateBound(node);
    if(NotNull(node.parent))
        propagateBoundToRoot(node.parent);
}
