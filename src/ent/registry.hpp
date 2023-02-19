#ifndef ENTT_ENTITY_REGISTRY_HPP
#define ENTT_ENTITY_REGISTRY_HPP

#include <stdexcept>
#include <vector>
#include <memory>
#include <utility>
#include <map>
#include <any>
#include <cstddef>
#include <cassert>
#include "family.hpp"
#include "sparse_set.hpp"
#include "view.hpp"

namespace entt
{

template<typename Entity>
class Registry
{
    using component_family = Family<struct InternalRegistryComponentFamily>;
    using traits_type      = entt_traits<Entity>;

    template<typename Component>
    using pool_instance = SparseSet<Entity, Component>;

    template<typename Component>
    bool managed() const noexcept
    {
        auto const ctype = component_family::type<Component>();
        return ctype < pools.size() && pools[ctype];
    }

    template<typename Component>
    pool_instance<Component> const & pool() const noexcept
    {
        assert(managed<Component>());
        return static_cast<pool_instance<Component> &>(*pools[component_family::type<Component>()]);
    }

    template<typename Component>
    pool_instance<Component> & pool() noexcept
    {
        assert(managed<Component>());
        return const_cast<pool_instance<Component> &>(const_cast<Registry const *>(this)->pool<Component>());
    }

    template<typename Component>
    pool_instance<Component> & ensure()
    {
        auto const ctype = component_family::type<Component>();

        if(!(ctype < pools.size()))
        {
            pools.resize(ctype + 1);
        }

        if(!pools[ctype])
        {
            pools[ctype] = std::make_unique<pool_instance<Component>>();
        }

        return pool<Component>();
    }

public:
    using entity_type  = typename traits_type::entity_type;
    using version_type = typename traits_type::version_type;
    using size_type    = std::size_t;

    explicit Registry() = default;
    ~Registry()         = default;

    Registry(Registry const &) = delete;
    Registry(Registry &&)      = delete;

    Registry & operator=(Registry const &) = delete;
    Registry & operator=(Registry &&)      = delete;

    template<typename Component>
    size_type size() const noexcept
    {
        return managed<Component>() ? pool<Component>().size() : size_type{};
    }

    size_type size() const noexcept { return entities.size() - available.size(); }

    size_type capacity() const noexcept { return entities.size(); }

    template<typename Component>
    bool empty() const noexcept
    {
        return managed<Component>() ? pool<Component>().empty() : true;
    }

    bool empty() const noexcept { return entities.size() == available.size(); }

    bool valid(entity_type entity) const noexcept
    {
        auto const entt = entity & traits_type::entity_mask;
        return (entt < entities.size() && entities[entt] == entity);
    }

    version_type version(entity_type entity) const noexcept
    {
        return version_type((entity >> traits_type::version_shift) & traits_type::version_mask);
    }

    version_type current(entity_type entity) const noexcept
    {
        auto const entt = entity & traits_type::entity_mask;
        assert(entt < entities.size());
        return version_type((entities[entt] >> traits_type::version_shift) & traits_type::version_mask);
    }

    template<typename... Component>
    entity_type create() noexcept
    {
        using accumulator_type       = int[];
        auto const       entity      = create();
        accumulator_type accumulator = {0, (ensure<Component>().construct(entity), 0)...};
        (void)accumulator;
        return entity;
    }

    entity_type create() noexcept
    {
        entity_type entity;

        if(available.empty())
        {
            entity = entity_type(entities.size());
            assert((entity >> traits_type::version_shift) == entity_type{});
            entities.push_back(entity);
        }
        else
        {
            entity = available.back();
            available.pop_back();
        }

        return entity;
    }

    void destroy(entity_type entity)
    {
        assert(valid(entity));

        auto const entt    = entity & traits_type::entity_mask;
        auto const version = 1 + ((entity >> traits_type::version_shift) & traits_type::version_mask);
        entities[entt]     = entt | (version << traits_type::version_shift);
        available.push_back(entities[entt]);

        for(auto && cpool : pools)
        {
            if(cpool && cpool->has(entity))
            {
                cpool->destroy(entity);
            }
        }
    }

    template<typename Component, typename... Args>
    Component & assign(entity_type entity, Args &&... args)
    {
        assert(valid(entity));
        return ensure<Component>().construct(entity, std::forward<Args>(args)...);
    }

    template<typename Component, typename... Args>
    Component & replace(entity_type entity, Args &&... args)
    {
        assert(valid(entity));
        return (pool<Component>().get(entity) = Component{std::forward<Args>(args)...});
    }

    template<typename Component, typename... Args>
    Component & add_component(entity_type entity, Args &&... args)
    {
        assert(valid(entity));

        return (has<Component>(entity) ? replace<Component>(entity, std::forward<Args>(args)...)
                                       : assign<Component>(entity, std::forward<Args>(args)...));
    }

    template<typename Component>
    void remove(entity_type entity)
    {
        assert(valid(entity));
        return pool<Component>().destroy(entity);
    }

    template<typename... Component>
    bool has(entity_type entity) const noexcept
    {
        static_assert(sizeof...(Component) > 0, "!");
        assert(valid(entity));

        using accumulator_type       = bool[];
        bool             all         = true;
        accumulator_type accumulator = {
            (all = all && managed<Component>() && pool<Component>().has(entity))...};
        (void)accumulator;
        return all;
    }

    template<typename Component>
    Component const & get(entity_type entity) const noexcept
    {
        assert(valid(entity));
        return pool<Component>().get(entity);
    }

    template<typename Component>
    Component & get(entity_type entity) noexcept
    {
        return const_cast<Component &>(const_cast<Registry const *>(this)->get<Component>(entity));
    }

    template<typename Component, typename Compare>
    void sort(Compare compare)
    {
        auto & cpool = ensure<Component>();

        cpool.sort([&cpool, compare = std::move(compare)](auto lhs, auto rhs) {
            return compare(static_cast<const Component &>(cpool.get(lhs)),
                           static_cast<const Component &>(cpool.get(rhs)));
        });
    }

    template<typename To, typename From>
    void sort()
    {
        ensure<To>().respect(ensure<From>());
    }

    template<typename Component>
    void reset(entity_type entity)
    {
        assert(valid(entity));

        if(managed<Component>())
        {
            auto & cpool = pool<Component>();

            if(cpool.has(entity))
            {
                cpool.destroy(entity);
            }
        }
    }

    template<typename Component>
    void reset()
    {
        if(managed<Component>())
        {
            auto & cpool = pool<Component>();

            cpool.reset();   /// !!!!!!!!!!!!!!!!!!!
            /*for(auto entity : entities)
            {
                if(cpool.has(entity))
                {
                    cpool.destroy(entity);
                }
            }*/
        }
    }

    void reset()
    {
        available.clear();
        pools.clear();

        for(auto && entity : entities)
        {
            auto const version = 1 + ((entity >> traits_type::version_shift) & traits_type::version_mask);
            entity = (entity & traits_type::entity_mask) | (version << traits_type::version_shift);
            available.push_back(entity);
        }
    }

    template<typename... Component>
    View<Entity, Component...> view()
    {
        return View<Entity, Component...>{ensure<Component>()...};
    }

    template<typename Type, typename... Args>
    Type & ctx_set(Args &&... args)
    {
        std::size_t type_id = component_family::type<Type>();
        context[type_id]    = std::any{std::in_place_type<Type>, std::forward<Args>(args)...};

        return std::any_cast<Type &>(context[type_id]);
    }

    template<typename Type>
    void ctx_unset()
    {
        std::size_t type_id = component_family::type<Type>();
        context.erase(type_id);
    }

    template<typename Type>
    Type & ctx_get()
    {
        std::size_t type_id = component_family::type<Type>();

        auto map_it = context.find(type_id);

        if(map_it != context.end() && map_it->second.has_value())
        {
            return std::any_cast<Type &>(map_it->second);
        }

        throw std::runtime_error("Try to get non existing object");
    }

private:
    std::vector<std::unique_ptr<SparseSet<Entity>>> pools;
    std::vector<entity_type>                        available;
    std::vector<entity_type>                        entities;
    std::map<std::size_t, std::any>                 context;
};

using DefaultRegistry = Registry<std::uint32_t>;

}   // namespace entt

#endif   // ENTT_ENTITY_REGISTRY_HPP
