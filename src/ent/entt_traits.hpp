#ifndef ENTT_ENTITY_ENTT_TRAITS_HPP
#define ENTT_ENTITY_ENTT_TRAITS_HPP

#include <cstdint>

namespace entt
{

template<typename>
struct entt_traits;

template<>
struct entt_traits<std::uint16_t>
{
    using entity_type     = std::uint16_t;
    using version_type    = std::uint8_t;
    using difference_type = std::int32_t;

    static constexpr auto entity_mask   = 0xFFF;
    static constexpr auto version_mask  = 0xF;
    static constexpr auto version_shift = 12;
};

template<>
struct entt_traits<std::uint32_t>
{
    using entity_type     = std::uint32_t;
    using version_type    = std::uint16_t;
    using difference_type = std::int64_t;

    static constexpr auto entity_mask   = 0xFFFFF;
    static constexpr auto version_mask  = 0xFFF;
    static constexpr auto version_shift = 20;
};

template<>
struct entt_traits<std::uint64_t>
{
    using entity_type     = std::uint64_t;
    using version_type    = std::uint32_t;
    using difference_type = std::int64_t;

    static constexpr auto entity_mask   = 0xFFFFFFFF;
    static constexpr auto version_mask  = 0xFFFFFFFF;
    static constexpr auto version_shift = 32;
};

}   // namespace entt

#endif   // ENTT_ENTITY_ENTT_TRAITS_HPP
