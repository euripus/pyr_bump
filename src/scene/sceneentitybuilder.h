#ifndef SCENEENTITYBUILDER_H
#define SCENEENTITYBUILDER_H

#include <bitset>

#include "../ent/registry.hpp"

using Entity = entt::DefaultRegistry::entity_type;

constexpr Entity null_entity_id = static_cast<Entity>(-1);

constexpr bool NotNull(Entity node_id) noexcept
{
    return node_id != null_entity_id;
}

struct ComponentFlagsBitsPos
{
    static int const pos      = 0;
    static int const cam      = 1;
    static int const light    = 2;
    static int const material = 3;
    static int const mesh     = 4;
};

using build_flags = std::bitset<5>;

constexpr build_flags pos_flags{0b00001};     // pos
constexpr build_flags cam_flags{0b00011};     // pos + cam
constexpr build_flags light_flags{0b00101};   // pos + light
constexpr build_flags obj_flags{0b11001};     // pos + material + mesh

class SceneEntityBuilder
{
public:
    static Entity BuildEntity(entt::DefaultRegistry & reg, build_flags flags);
};

struct ISystem
{
    virtual bool        init()                                                       = 0;
    virtual void        update(entt::DefaultRegistry & reg, float time_delta = 1.0f) = 0;
    virtual std::string getName() const                                              = 0;
};

#endif   // SCENEENTITYBUILDER_H
