#include "sceneentitybuilder.h"
#include "scene.h"
#include "camera.h"

Entity SceneEntityBuilder::BuildEntity(Registry & reg, build_flags flags)
{
    Entity entity = null_entity_id;

    if(flags[ComponentFlagsBitsPos::pos])
    {
        entity = reg.create();

        auto scene                               = evnt::SceneSystem::GetDefaultSceneComponent();
        scene.entity_id                          = entity;
        reg.assign<evnt::SceneComponent>(entity) = scene;
    }
    if(flags[ComponentFlagsBitsPos::cam])
    {
        if(entity == null_entity_id)
            entity = reg.create();

        auto cam                            = CameraSystem::GetDefaultCamComponent();
        cam.m_entity_id                     = entity;
        reg.assign<CameraComponent>(entity) = cam;
    }

    return entity;
}
