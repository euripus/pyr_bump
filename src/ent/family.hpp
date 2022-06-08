#ifndef ENTT_CORE_FAMILY_HPP
#define ENTT_CORE_FAMILY_HPP

#include <type_traits>
#include <cstddef>
#include <atomic>

namespace entt
{

template<typename...>
class Family
{
    static std::atomic<std::size_t> identifier;

    template<typename...>
    static std::size_t family() noexcept
    {
        static const std::size_t value = identifier.fetch_add(1);
        return value;
    }

public:
    using family_type = std::size_t;

    template<typename... Type>
    inline static family_type type() noexcept
    {
        return family<std::decay_t<Type>...>();
    }
};

template<typename... Types>
std::atomic<std::size_t> Family<Types...>::identifier{};

}   // namespace entt

#endif   // ENTT_CORE_FAMILY_HPP
