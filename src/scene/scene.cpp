#include "scene.h"
#include "model.h"
#include <stdexcept>

SceneComponent SceneSystem::GetDefaultSceneComponent()
{
    SceneComponent node;
    // set defaults
    node.abs       = glm::mat4(1.0f);
    node.rel       = glm::mat4(1.0f);
    node.entity_id = null_entity_id;
    node.parent    = null_entity_id;

    return node;
}

void SceneSystem::update(double time)
{
    for(auto ent : m_reg.view<SceneComponent, Event::Model::CreateModel>())
    {
        auto & cm_event = m_reg.get<Event::Model::CreateModel>(ent);
        auto & pos      = m_reg.get<SceneComponent>(ent);

        pos.rel = cm_event.rel_transform;

        connectNode(ent, cm_event.parent);
    }

    // update changed Bboxes from joint sysytem upate call
    for(auto ent : m_reg.view<SceneComponent, Event::Scene::IsBboxUpdated>())
    {
        auto & pos = m_reg.get<SceneComponent>(ent);

        updateBound(ent);

        if(NotNull(pos.parent) && pos.transformed_bbox)
            propagateBoundToRoot(pos.parent);
    }
    m_reg.reset<Event::Scene::IsBboxUpdated>();

    for(auto ent : m_reg.view<SceneComponent, Event::Scene::TransformComponent>())
    {
        m_transform_updated = true;

        auto & trans = m_reg.get<Event::Scene::TransformComponent>(ent);
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
        m_reg.reset<Event::Scene::TransformComponent>();

    for(auto ent : m_reg.view<SceneComponent, Event::Model::DestroyModel>())
    {
        disconnectNode(ent);
    }
}

void SceneSystem::postUpdate()
{
    if(m_transform_updated)
    {
        m_reg.reset<Event::Scene::IsTransformed>();
        m_transform_updated = false;
    }
}

void SceneSystem::connectNode(Entity node_id, Entity parent)
{
    auto & node = m_reg.get<SceneComponent>(node_id);

    if(parent == null_entity_id)
    {
        if(NotNull(m_root))
        {
            throw std::runtime_error{"Error, trying to add a new root to an existing scene!"};
        }

        m_root = node_id;
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

void SceneSystem::disconnectNode(Entity node_id)
{
    auto & node = m_reg.get<SceneComponent>(node_id);

    if(NotNull(node.parent))
    {
        auto & parent_node = m_reg.get<SceneComponent>(node.parent);

        parent_node.children.remove(node_id);
        updateBound(node.parent);
        node.parent = null_entity_id;

        if(NotNull(parent_node.parent))
            propagateBoundToRoot(parent_node.parent);
    }
}

void SceneSystem::updateQueues(evnt::Frustum const & frustum1, evnt::Frustum const * frustum2)
{
    m_models_queue.resize(0);

    updateQueuesRec(frustum1, frustum2, m_root);
}

void SceneSystem::updateQueuesRec(evnt::Frustum const & frustum1, evnt::Frustum const * frustum2,
                                  Entity node_id)
{
    auto const & node = m_reg.get<SceneComponent>(node_id);

    if(node.transformed_bbox && frustum1.cullBox(*node.transformed_bbox))
    {
        if(frustum2 == nullptr || frustum2->cullBox(*node.transformed_bbox))
            return;
    }

    // mesh nodes
    if(m_reg.has<ModelComponent>(node_id) && node.transformed_bbox
       && !frustum1.cullBox(*node.transformed_bbox))
    {
        if(frustum2 == nullptr || !frustum2->cullBox(*node.transformed_bbox))
            m_models_queue.push_back(node_id);
    }

    for(auto ch : node.children)
    {
        updateQueuesRec(frustum1, frustum2, ch);
    }
}

void SceneSystem::updateTransform(Entity node_id, bool initiator)
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
    m_reg.add_component<Event::Scene::IsTransformed>(node_id);

    for(auto ch : node.children)
    {
        updateTransform(ch, false);
    }

    updateBound(node_id);

    if(initiator && NotNull(node.parent) && node.transformed_bbox)
        propagateBoundToRoot(node.parent);
}

void SceneSystem::updateBound(Entity node_id)
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
                node.transformed_bbox = evnt::AABB();

            node.transformed_bbox->expandBy(*child_node.transformed_bbox);
        }
    }
}

void SceneSystem::propagateBoundToRoot(Entity parent_node_id)
{
    updateBound(parent_node_id);

    auto const & node = m_reg.get<SceneComponent>(parent_node_id);
    if(NotNull(node.parent))
        propagateBoundToRoot(node.parent);
}
