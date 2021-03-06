#include "sceneentitybuilder.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "model.h"

Entity SceneEntityBuilder::BuildEntity(Registry & reg, build_flags flags)
{
    Entity entity = null_entity_id;

    if(flags[ComponentFlagsBitsPos::pos])
    {
        entity = reg.create();

        auto scene      = evnt::SceneSystem::GetDefaultSceneComponent();
        scene.entity_id = entity;
        reg.assign<evnt::SceneComponent>(entity, scene);
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
    if(flags[ComponentFlagsBitsPos::mesh])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        auto mdl = ModelSystem::GetDefaultModelComponent();
        reg.assign<ModelComponent>(entity, mdl);
    }

    return entity;
}

void SystemsMgr::addSystem(std::unique_ptr<ISystem> sys_ptr)
{
    m_systems.push_back(std::move(sys_ptr));
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

void SystemsMgr::update(float time_delta)
{
    for(auto & sys : m_systems)
    {
        sys->update(time_delta);
    }

    for(auto & sys : m_systems)
    {
        sys->postUpdate();
    }
}
