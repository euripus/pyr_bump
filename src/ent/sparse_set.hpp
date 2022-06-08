#ifndef ENTT_ENTITY_SPARSE_SET_HPP
#define ENTT_ENTITY_SPARSE_SET_HPP

#include <algorithm>
#include <utility>
#include <vector>
#include <cstddef>
#include <cassert>
#include "entt_traits.hpp"

namespace entt
{

template<typename...>
class SparseSet;

template<typename Entity>
class SparseSet<Entity>
{
    using traits_type = entt_traits<Entity>;

    struct Iterator
    {
        using value_type = Entity;

        Iterator(std::vector<Entity> const * direct, std::size_t pos) : direct{direct}, pos{pos} {}

        Iterator & operator++() noexcept { return --pos, *this; }

        Iterator operator++(int) noexcept
        {
            Iterator orig = *this;
            return ++(*this), orig;
        }

        bool operator==(Iterator const & other) const noexcept
        {
            return other.pos == pos && other.direct == direct;
        }

        bool operator!=(Iterator const & other) const noexcept { return !(*this == other); }

        value_type operator*() const noexcept { return (*direct)[pos - 1]; }

    private:
        std::vector<Entity> const * direct;
        std::size_t                 pos;
    };

public:
    using entity_type   = Entity;
    using pos_type      = entity_type;
    using size_type     = std::size_t;
    using iterator_type = Iterator;

    explicit SparseSet() noexcept = default;

    SparseSet(SparseSet const &) = delete;
    SparseSet(SparseSet &&)      = default;

    SparseSet & operator=(SparseSet const &) = delete;
    SparseSet & operator=(SparseSet &&)      = default;

    virtual ~SparseSet() noexcept = default;

    size_type size() const noexcept { return direct.size(); }

    bool empty() const noexcept { return direct.empty(); }

    entity_type const * data() const noexcept { return direct.data(); }

    iterator_type begin() const noexcept { return Iterator{&direct, direct.size()}; }

    iterator_type end() const noexcept { return Iterator{&direct, 0}; }

    bool has(entity_type entity) const noexcept
    {
        auto const entt = entity & traits_type::entity_mask;

        return entt < reverse.size() && reverse[entt] < direct.size() && direct[reverse[entt]] == entity;
    }

    pos_type get(entity_type entity) const noexcept
    {
        assert(has(entity));
        return reverse[entity & traits_type::entity_mask];
    }

    pos_type construct(entity_type entity)
    {
        assert(!has(entity));
        auto const entt = entity & traits_type::entity_mask;

        if(!(entt < reverse.size()))
        {
            reverse.resize(entt + 1);
        }

        auto const pos = pos_type(direct.size());
        reverse[entt]  = pos;
        direct.emplace_back(entity);

        return pos;
    }

    virtual void destroy(entity_type entity)
    {
        assert(has(entity));
        auto const entt = entity & traits_type::entity_mask;
        auto const back = direct.back() & traits_type::entity_mask;
        auto const pos  = reverse[entt];

        reverse[back] = pos;
        direct[pos]   = direct.back();
        direct.pop_back();
    }

    virtual void swap(entity_type lhs, entity_type rhs)
    {
        assert(has(lhs));
        assert(has(rhs));
        auto const le = lhs & traits_type::entity_mask;
        auto const re = rhs & traits_type::entity_mask;

        std::swap(direct[reverse[le]], direct[reverse[re]]);
        std::swap(reverse[le], reverse[re]);
    }

    template<typename Compare>
    void sort(Compare compare)
    {
        std::vector<pos_type> copy{direct.cbegin(), direct.cend()};
        std::sort(copy.begin(), copy.end(),
                  [compare = std::move(compare)](auto... args) { return !compare(args...); });

        for(pos_type i = 0; i < copy.size(); ++i)
        {
            if(direct[i] != copy[i])
            {
                swap(direct[i], copy[i]);
            }
        }
    }

    void respect(SparseSet<Entity> const & other)
    {
        struct Bool
        {
            bool value{false};
        };

        std::vector<Bool> check(std::max(other.reverse.size(), reverse.size()));

        for(auto entity : other.direct)
        {
            check[entity & traits_type::entity_mask].value = true;
        }

        sort([this, &other, &check](auto lhs, auto rhs) {
            const auto le = lhs & traits_type::entity_mask;
            const auto re = rhs & traits_type::entity_mask;

            const bool bLhs    = check[le].value;
            const bool bRhs    = check[re].value;
            bool       compare = false;

            if(bLhs && bRhs)
            {
                compare = other.get(rhs) < other.get(lhs);
            }
            else if(!bLhs && !bRhs)
            {
                compare = re < le;
            }
            else
            {
                compare = bLhs;
            }

            return compare;
        });
    }

    virtual void reset()
    {
        reverse.clear();
        direct.clear();
    }

private:
    std::vector<entity_type> reverse;
    std::vector<entity_type> direct;
};

template<typename Entity, typename Type>
class SparseSet<Entity, Type> : public SparseSet<Entity>
{
    using base_type = SparseSet<Entity>;

public:
    using type          = Type;
    using entity_type   = typename base_type::entity_type;
    using pos_type      = typename base_type::pos_type;
    using size_type     = typename base_type::size_type;
    using iterator_type = typename base_type::iterator_type;

    explicit SparseSet() noexcept = default;

    SparseSet(SparseSet const &) = delete;
    SparseSet(SparseSet &&)      = default;

    SparseSet & operator=(SparseSet const &) = delete;
    SparseSet & operator=(SparseSet &&)      = default;

    type const * raw() const noexcept { return instances.data(); }

    type * raw() noexcept { return instances.data(); }

    type const & get(entity_type entity) const noexcept { return instances[base_type::get(entity)]; }

    type & get(entity_type entity) noexcept
    {
        return const_cast<type &>(const_cast<SparseSet const *>(this)->get(entity));
    }

    template<typename... Args>
    type & construct(entity_type entity, Args &&... args)
    {
        base_type::construct(entity);
        instances.push_back({std::forward<Args>(args)...});
        return instances.back();
    }

    void destroy(entity_type entity) override
    {
        instances[base_type::get(entity)] = std::move(instances.back());
        instances.pop_back();
        base_type::destroy(entity);
    }

    void swap(entity_type lhs, entity_type rhs) override
    {
        std::swap(instances[base_type::get(lhs)], instances[base_type::get(rhs)]);
        base_type::swap(lhs, rhs);
    }

    void reset() override
    {
        base_type::reset();
        instances.clear();
    }

private:
    std::vector<type> instances;
};

}   // namespace entt

#endif   // ENTT_ENTITY_SPARSE_SET_HPP
