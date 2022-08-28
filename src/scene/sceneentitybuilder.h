#ifndef SCENEENTITYBUILDER_H
#define SCENEENTITYBUILDER_H

#include <bitset>
#include <vector>
#include <memory>

#include "../ent/registry.hpp"

using Entity   = entt::DefaultRegistry::entity_type;
using Registry = entt::DefaultRegistry;

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
    static int const model    = 4;
    static int const joint    = 5;
};

using build_flags = std::bitset<6>;

constexpr build_flags pos_flags   = 0b000001;   // pos
constexpr build_flags cam_flags   = 0b000011;   // pos + cam
constexpr build_flags light_flags = 0b000101;   // pos + light
constexpr build_flags obj_flags   = 0b011001;   // pos + material + model
constexpr build_flags joint_flags = 0b100001;   // pos + joint

class SceneEntityBuilder
{
public:
    static Entity BuildEntity(Registry & reg, build_flags flags);
};

struct ISystem
{
    ISystem(Registry & reg) : m_reg(reg) {}
    virtual ~ISystem() = default;

    virtual bool        init() { return true; }
    virtual void        update(float time_delta = 1.0f) = 0;
    virtual void        postUpdate() {}
    virtual std::string getName() const = 0;

    Registry & getRegistry() const { return m_reg; }

protected:
    Registry & m_reg;
};

class SystemsMgr
{
    std::vector<std::shared_ptr<ISystem>> m_systems;
    Registry &                            m_reg;
public:
    SystemsMgr(Registry & reg) : m_reg(reg) {}

    void addSystem(std::shared_ptr<ISystem> sys_ptr);

    bool initSystems();
    void update(float time_delta = 1.0f);
};

#endif   // SCENEENTITYBUILDER_H
