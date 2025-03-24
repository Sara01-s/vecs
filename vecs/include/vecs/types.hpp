#pragma once

// std
#include <bitset>
#include <cstdint>
#include <type_traits>
#include <typeinfo>
#include <variant>

namespace vecs {

// Primitive types.
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

#ifdef __SIZEOF_INT128__
using u128 = __uint128_t;
using i128 = __int128_t;
#else
    #error "128-bit integers are not supported on this platform."
#endif

using f32 = float;
using f64 = double;

#ifdef __SIZEOF_FLOAT128__
using f128 = long double;
#else
    #error "128-bit floats are not supported on this platform."
#endif

using usize = std::size_t;
using isize = std::ptrdiff_t;
using uptr = std::uintptr_t;
using iptr = std::intptr_t;

using c8 = char;
using uc8 = unsigned char;
using sc8 = signed char;
using wc16 = wchar_t; // wide chars such as L'Ω'. Omega (U+03A9).

// ECS Types. ////////////////////////////////////////////////////////////
static constexpr vecs::usize MAX_ALIVE_ENTITIES {16}; // Arbitrarily set.
static constexpr vecs::usize MAX_REGISTRABLE_COMPONENTS {8}; // Arbitrarily set.
static constexpr vecs::usize MAX_INSTANCES_PER_COMPONENT {64
}; // Arbitrarily set.
static constexpr vecs::usize DEAD_ENTITY_ID {0b0
}; // 0b0 = No components = dead/invalid entity.

/* Vanila ECS definition (informal, by: Sander Mertens, 2020).
   src: https://ajmmertens.medium.com/why-vanilla-ecs-is-not-enough-d7ed4e3bebe5

    - An entity is a unique identifier.
    - A component is a plain old data type.
    - Each entity is associated with 0..N components.
    - A system is logic matched with entities based on their components.
*/
using entity_id_t = vecs::u64;

// Note: Component names are mangled. (src:
// https://en.cppreference.com/w/cpp/types/type_info/name)
using component_name_t = decltype(std::declval<std::type_info>().name());
using component_id_t = decltype(std::declval<std::type_info>().hash_code());
using component_key_t =
    vecs::u64; // See slotmap.hpp. (search for key_t definition).

// Restrictions for a type to be considered as a Component.
template <typename T>
concept Component = !std::is_polymorphic_v<T> && // Components must be concrete.
    std::is_trivially_constructible_v<T> && // Components must not have custom
    // constructors.
    std::is_trivially_copyable_v<T> && // Components must not have custom copy
    // constructors.
    std::is_trivially_move_assignable_v<T> && // Components must not have custom
    // move constructors.
    std::is_trivially_destructible_v<T>; // Components must not have custom
// destructors.

/* A component can only be an instance of a component
    This implies that all components must be known at compile time.
    Note:
         A std::variant is similar to a C union, but it is type-safe.
         Its size is the maximum size of all the alternative types in the
   variant. (e.g.), sizeof(component_t) = max(sizeof(Position),
   sizeof(Velocity), sizeof(Health)).
*/
template <Component... Cs>
using component_t = std::variant<Cs...>;

using mask_t = std::bitset<MAX_REGISTRABLE_COMPONENTS>;

template <typename T>
concept ScheduleLabel = std::is_empty_v<T>;

struct world_t;
using schedule_label_id_t =
    decltype(std::declval<std::type_info>().hash_code());
using system_t = void (*)(vecs::world_t&);

} // namespace vecs
