#include "scene.h"

evnt::SceneComponent evnt::SceneSystem::GetDefaultSceneComponent()
{
    evnt::SceneComponent node;
    // set defaults
    node.abs       = glm::mat4(1.0f);
    node.rel       = glm::mat4(1.0f);
    node.entity_id = null_entity_id;
    node.parent    = null_entity_id;

    return node;
}

void evnt::SceneSystem::update(double time)
{
    // update changed Bboxes from joint sysytem upate call
    for(auto ent : m_reg.view<SceneComponent, IsBboxUpdated>())
    {
        auto & pos = m_reg.get<SceneComponent>(ent);

        updateBound(ent);

        if(NotNull(pos.parent) && pos.transformed_bbox)
            propagateBoundToRoot(pos.parent);
    }

    m_reg.reset<IsBboxUpdated>();

    for(auto ent : m_reg.view<SceneComponent, TransformComponent>())
    {
        m_transform_updated = true;

        auto & trans = m_reg.get<TransformComponent>(ent);
        auto & pos   = m_reg.get<SceneComponent>(ent);

        if(trans.replase_local_matrix)
            pos.rel = trans.new_mat;
        else
            // old transformation first M_new * M_old * vtx
            pos.rel = trans.new_mat * pos.rel;

        updateTransform(ent, true);
    }

    // clear all TransformComponent
    if(m_transform_updated)
        m_reg.reset<TransformComponent>();
}

void evnt::SceneSystem::postUpdate()
{
    if(m_transform_updated)
    {
        m_reg.reset<evnt::IsTransformed>();
        m_transform_updated = false;
    }
}

void evnt::SceneSystem::addNode(Entity node_id, Entity parent)
{
    auto & node = m_reg.get<SceneComponent>(node_id);

    if(parent == null_entity_id)
    {
        updateBound(node_id);
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

void evnt::SceneSystem::updateTransform(Entity node_id, bool initiator)
{
    auto & node = m_reg.get<SceneComponent>(node_id);

    if(NotNull(node.parent))
    {
        auto & parent_node = m_reg.get<SceneComponent>(node.parent);
        node.abs           = parent_node.abs * node.rel;
    }
    else
    {
        node.abs = node.rel;
    }
    // mark entity
    m_reg.add_component<evnt::IsTransformed>(node_id);

    for(auto ch : node.children)
    {
        updateTransform(ch, false);
    }

    updateBound(node_id);

    if(initiator && NotNull(node.parent) && node.transformed_bbox)
        propagateBoundToRoot(node.parent);
}

void evnt::SceneSystem::updateBound(Entity node_id)
{
    auto & node = m_reg.get<SceneComponent>(node_id);

    if(node.initial_bbox)
    {
        node.transformed_bbox = node.initial_bbox;
        node.transformed_bbox->transform(node.abs);
    }
    else
    {
        if(node.transformed_bbox)
            node.transformed_bbox.reset();
    }

    // expand from children
    for(auto ch : node.children)
    {
        auto & child_node = m_reg.get<SceneComponent>(ch);

        if(child_node.transformed_bbox)
        {
            if(!node.transformed_bbox)
                node.transformed_bbox = AABB();

            node.transformed_bbox->expandBy(*child_node.transformed_bbox);
        }
    }
}

void evnt::SceneSystem::propagateBoundToRoot(Entity parent_node_id)
{
    auto & node = m_reg.get<SceneComponent>(parent_node_id);

    if(!node.transformed_bbox)
        node.transformed_bbox = AABB();

    for(auto ch : node.children)
    {
        auto & child_node = m_reg.get<SceneComponent>(ch);

        if(child_node.transformed_bbox)
        {
            node.transformed_bbox->expandBy(*child_node.transformed_bbox);
        }
    }

    if(NotNull(node.parent))
        propagateBoundToRoot(node.parent);
}
