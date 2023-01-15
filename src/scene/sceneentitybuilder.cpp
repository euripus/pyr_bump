#include "sceneentitybuilder.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "model.h"
#include <algorithm>

Entity EntityBuilder::BuildEntity(Registry & reg, build_flags flags)
{
    Entity entity = null_entity_id;

    if(flags[ComponentFlagsBitsPos::pos])
    {
        entity = reg.create();

        auto scene      = SceneSystem::GetDefaultSceneComponent();
        scene.entity_id = entity;
        reg.assign<SceneComponent>(entity, scene);
    }
    if(flags[ComponentFlagsBitsPos::cam])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        auto cam        = CameraSystem::GetDefaultCamComponent();
        cam.m_entity_id = entity;
        reg.assign<CameraComponent>(entity, cam);
    }
    if(flags[ComponentFlagsBitsPos::light])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        auto light = LightSystem::GetDefaultLightComponent();
        reg.assign<LightComponent>(entity, light);
    }
    if(flags[ComponentFlagsBitsPos::material])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        auto mat = MaterialSystem::GetDefaultMaterialComponent();
        reg.assign<MaterialComponent>(entity, std::move(mat));
    }
    if(flags[ComponentFlagsBitsPos::model])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        auto mdl = ModelSystem::GetDefaultModelComponent();
        reg.assign<ModelComponent>(entity, mdl);
    }
    if(flags[ComponentFlagsBitsPos::joint])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        reg.assign<JointComponent>(entity);
    }

    return entity;
}

void EntityBuilder::DestroyEntity(Registry & reg, Entity entity_id)
{
    reg.destroy(entity_id);
}

SystemsMgr::~SystemsMgr()
{
    // found unique systems, in m_system may be dublicates
    std::vector<ISystem *> unique_ptrs;
    for(auto & shd_ptr : m_systems)
    {
        ISystem * cur_ptr = shd_ptr.get();
        if(std::find(begin(unique_ptrs), end(unique_ptrs), cur_ptr) == std::end(unique_ptrs))
            unique_ptrs.push_back(cur_ptr);
    }

    for(auto * ptr : unique_ptrs)
        ptr->terminate();
}

void SystemsMgr::addSystem(std::shared_ptr<ISystem> sys_ptr)
{
    m_systems.push_back(sys_ptr);
}

bool SystemsMgr::initSystems()
{
    for(auto & sys : m_systems)
    {
        if(!sys->init())
            return false;
    }

    return true;
}

void SystemsMgr::update(double time)
{
    for(auto & sys : m_systems)
    {
        sys->update(time);
    }

    for(auto & sys : m_systems)
    {
        sys->postUpdate();
    }
}

void EntityDeleterSystem::update(double time)
{
    std::vector<Entity> entities;
    for(auto ent : m_reg.view<Event::Deleter::DeleteEntity>())
    {
        entities.push_back(ent);
    }

    if(!entities.empty())
    {
        for(auto ent : entities)
            EntityBuilder::DestroyEntity(m_reg, ent);
    }
}

void EntityCreatorSystem::update(double time)
{
    if(m_create_queue.empty())
        return;

    for(auto & func : m_create_queue)
        func();

    m_create_queue.resize(0);
}
