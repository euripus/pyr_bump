#ifndef ENTT_ENTITY_VIEW_HPP
#define ENTT_ENTITY_VIEW_HPP

#include <tuple>
#include "sparse_set.hpp"

namespace entt
{

template<typename Entity, typename First, typename... Other>
class View final
{
    template<typename Component>
    using pool_type = SparseSet<Entity, Component>;

    using base_pool_type           = SparseSet<Entity>;
    using underlying_iterator_type = typename base_pool_type::iterator_type;
    using repo_type                = std::tuple<pool_type<First> &, pool_type<Other> &...>;

    class Iterator
    {
        inline bool valid() const noexcept
        {
            using accumulator_type = bool[];
            auto entity            = *begin;
            bool all               = std::get<pool_type<First> &>(pools).has(entity);

            accumulator_type accumulator = {
                (all = all && std::get<pool_type<Other> &>(pools).has(entity))...};
            (void)accumulator;
            return all;
        }

    public:
        using value_type = typename base_pool_type::entity_type;

        Iterator(repo_type const & pools, underlying_iterator_type begin,
                 underlying_iterator_type end) noexcept :
            pools{pools}, begin{begin}, end{end}
        {
            if(begin != end && !valid())
            {
                ++(*this);
            }
        }

        Iterator & operator++() noexcept
        {
            ++begin;
            while(begin != end && !valid())
            {
                ++begin;
            }
            return *this;
        }

        Iterator operator++(int) noexcept
        {
            Iterator orig = *this;
            return ++(*this), orig;
        }

        bool operator==(Iterator const & other) const noexcept { return other.begin == begin; }

        bool operator!=(Iterator const & other) const noexcept { return !(*this == other); }

        value_type operator*() const noexcept { return *begin; }

    private:
        repo_type const &        pools;
        underlying_iterator_type begin;
        underlying_iterator_type end;
    };

public:
    using iterator_type = Iterator;
    using entity_type   = typename base_pool_type::entity_type;
    using size_type     = typename base_pool_type::size_type;

    explicit View(pool_type<First> & pool, pool_type<Other> &... other) noexcept :
        pools{pool, other...}, view{nullptr}
    {
        reset();
    }

    iterator_type begin() const noexcept { return Iterator{pools, view->begin(), view->end()}; }

    iterator_type end() const noexcept { return Iterator{pools, view->end(), view->end()}; }

    template<typename Component>
    Component const & get(entity_type entity) const noexcept
    {
        return std::get<pool_type<Component> &>(pools).get(entity);
    }

    template<typename Component>
    Component & get(entity_type entity) noexcept
    {
        return const_cast<Component &>(const_cast<View const *>(this)->get<Component>(entity));
    }

    void reset()
    {
        using accumulator_type       = void *[];
        view                         = &std::get<pool_type<First> &>(pools);
        accumulator_type accumulator = {(std::get<pool_type<Other> &>(pools).size() < view->size()
                                             ? (view = &std::get<pool_type<Other> &>(pools))
                                             : nullptr)...};
        (void)accumulator;
    }

private:
    repo_type        pools;
    base_pool_type * view;
};

template<typename Entity, typename Component>
class View<Entity, Component> final
{
    using pool_type = SparseSet<Entity, Component>;

public:
    using iterator_type = typename pool_type::iterator_type;
    using entity_type   = typename pool_type::entity_type;
    using size_type     = typename pool_type::size_type;
    using raw_type      = typename pool_type::type;

    explicit View(pool_type & pool) noexcept : pool{pool} {}

    size_type size() const noexcept { return pool.size(); }

    raw_type * raw() noexcept { return pool.raw(); }

    raw_type const * raw() const noexcept { return pool.raw(); }

    entity_type const * data() const noexcept { return pool.data(); }

    iterator_type begin() const noexcept { return pool.begin(); }

    iterator_type end() const noexcept { return pool.end(); }

    Component const & get(entity_type entity) const noexcept { return pool.get(entity); }

    Component & get(entity_type entity) noexcept
    {
        return const_cast<Component &>(const_cast<View const *>(this)->get(entity));
    }

private:
    pool_type & pool;
};

}   // namespace entt

#endif   // ENTT_ENTITY_VIEW_HPP
