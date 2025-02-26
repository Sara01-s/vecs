#pragma once

// std
#include <iostream> // TODO - Delete this.
#include <cstdint>
#include <variant>
#include <typeinfo>
#include <type_traits>
#include <cassert>
// Replace with dense_map in the future.
// Or use a robin hood hash map.
#include <unordered_map>
#include <vector>
#include <bitset>
#include <numeric>
#include <tuple>
#include <concepts>

// lib
#include "data_structures/resusable_id.hpp"
#include "types.hpp"

namespace vecs {

/* Vanila ECS definition (informal, by: Sander Mertens, 2020).
   src: https://ajmmertens.medium.com/why-vanilla-ecs-is-not-enough-d7ed4e3bebe5

    - An entity is a unique identifier.
    - A component is a plain old data type.
    - Each entity is associated with 0..N components.
    - A system is logic matched with entities based on their components.
*/
using entity_id_t = vecs::u64;

// Note: Component names are mangled. (src: https://en.cppreference.com/w/cpp/types/type_info/name)
using component_name_t = decltype(std::declval<std::type_info>().name());

// Component identifies can be used as entity ids.
// Based on the proposal by Sander Mertens, creator of flecs library.
// src: https://ajmmertens.medium.com/why-vanilla-ecs-is-not-enough-d7ed4e3bebe5
using component_id_t = entity_id_t;

// MAX_REGISTRABLE_COMPONENTS is defined as 64 to fit inside a 64-bit bitset.
static constexpr vecs::usize MAX_REGISTRABLE_COMPONENTS { 64 };
static constexpr vecs::usize MAX_ALIVE_ENTITIES { 1'000 }; // Arbitrarily set.

template <typename T>
concept Component = 
    !std::is_polymorphic_v<T> &&                // Components must be concrete.
    std::is_trivially_constructible_v<T> &&     // Components must not have custom constructors.
    std::is_trivially_copyable_v<T> &&          // Components must not have custom copy constructors.
    std::is_trivially_move_assignable_v<T> &&   // Components must not have custom move constructors.
    std::is_trivially_destructible_v<T>;        // Components must not have custom destructors.

/* A component can be one and only one of the registered components.

    This implies that all components must be known at compile time.
    Note: 
         A std::variant is similar to a C union, but it is type-safe.
         Its size is the maximum size of all the alternative types in the variant.
         (e.g.), sizeof(component_t) = max(sizeof(Position), sizeof(Velocity), sizeof(Health)).
*/
template <Component... RegisteredComponents>
using component_t = std::variant<RegisteredComponents...>;

using mask_t = std::bitset<MAX_REGISTRABLE_COMPONENTS>;

template <Component... Cs>
struct archetype_t final {
    constexpr vecs::usize
    entity_count() const noexcept {
        return std::get<0>(component_table).size();
    }

    template <Component T>
    T& 
    get_component(vecs::usize index) {
        auto const& components = std::get<component_row_t<T>>(component_table);
        return components[index];
    }

    template <typename F>
    void 
    for_each(F&& function) {
        assert(!std::get<0>(component_table).empty());

        vecs::usize const entity_count { std::get<0>(component_table).size() };

        for (vecs::usize i{}; i < entity_count; ++i) {
            std::apply([&function, i](auto&... rows) {
                std::forward<F>(function)(rows[i]...);
            }, component_table);
        }
    }

    /* An archetype is identified using unique bitset of component masks.
        (e.g.):
            PositionMask = 0b0001
            VelocityMask = 0b0010
            HealthMask   = 0b0100

            (| = bitwise OR operator)
            Archetype (A) = PositionMask | VelocityMask | HealthMask = 0b0111
            Archetype (B) = PositionMask | VelocityMask              = 0b0011
            Archetype (C) = VelocityMask | PositionMask              = 0b0011
            Archetype (D) = PositionMask                             = 0b0001
        Note:
            B == C (order or components does not affect the mask).
    */
    mask_t mask{};

    /*  Columns = Entity ids.
        Rows    = Components Types.

        Archetype Visualization (Mask is 64-bit, truncated here for brevity):
        +---------------------------------------------------+
        |                      ARCHETYPE                    |                              
        +-----------+---------------------------------------+
        |    Mask   |  ...0000 0000 0000 0000 0000 0111     | 
        +-----------+------------+-------------+------------+
        |   Table   |  Entity 0  |   Entity 1  |  Entity 2  |
        +-----------+------------+-------------+------------+
        | Position  | (0.0, 0.0) | (-1.0, 2.0) | (3.2, 1.0) |
        | Velocity  | (0.0, 0.0) | (1.0, 1.0)  | (2.3, 0.0) |
        | Health    | 100        | 42          | 7          |
        +-----------+------------+-------------+------------+
    */
    template <Component T>
    using component_row_t = std::vector<T>;
    using component_table_t = std::tuple<component_row_t<Cs>...>;
    component_table_t component_table{};

    std::unordered_map<entity_id_t, component_table_t> entity_components{};
};


// T... = Registered Components.
// Example usage: `component_storage_t<Position, Velocity, Health> component_storage;`
template <Component... RegisteredComponents>
struct component_storage_t final {
    // Since Component concept (see RegisteredComponents declaration) guarantees that a component is NOT polymorphic:
    // `typeid(component)` is resolved at *compile time*.
    // src: https://en.cppreference.com/w/cpp/language/typeid#:~:text=Notes,is%20resolved%20at%20compile%20time.
    component_storage_t() 
    : _registered_components_type_info { &typeid(RegisteredComponents)... } 
    {
        static_assert(s_registered_components_size > 0, 
            "Component storage must have at least 1 registered component.");
        
        register_components(_registered_components_type_info);
    }

    template <Component... Cs>
    void 
    spawn_entity(Cs&&... components) {
        auto const& archetype = register_archetype(std::forward<Cs...>(components...));
        std::cout << archetype.entity_count() << std::endl;
    }
    
    constexpr vecs::usize
    get_registered_component_count() const noexcept {
        return _registered_components_count;
    }

private:
    void
    register_components(auto const& components_type_info) noexcept {
        for (auto const component_info: components_type_info) {
            static vecs::u64 s_next_component_mask = 0b1; // Marked as static because only used in this scope.
            assert(s_next_component_mask <= std::numeric_limits<decltype(s_next_component_mask)>::max()
                && "Component registration limit reached.");

            component_id_t const component_id = component_info->hash_code();
            
            assert(!_component_masks.contains(component_id) 
                && "Component already registered.");
            
            _component_masks[component_id] = s_next_component_mask;
            s_next_component_mask <<= 1;

            _registered_components_count += 1;
            
            // TODO - Remove this log.
            component_name_t const component_name = component_info->name();
            std::printf("%s -> %s\n", component_name, _component_masks[component_id].to_string().c_str());
        }
    }

    template <Component... Cs>
    [[nodiscard]] archetype_t<Cs...>
    register_archetype(Cs&&... components) noexcept {
        static_assert(sizeof...(Cs) > 0, 
            "Archetype must have at least 1 associated component.");
        
        archetype_t<Cs...> new_archetype{};

        // Create Archetype Mask.
        // C++17 fold expression for iterating variadic components.
        // src: https://en.cppreference.com/w/cpp/language/fold#:~:text=%7D-,//%20Using%20an%20integer%20sequence,%7D,-constexpr%20auto%20bswap(std::unsigned_integral
        ([&] (auto const& component) {
            component_id_t component_id = typeid(component).hash_code();
            mask_t component_mask = _component_masks[component_id];
            
            new_archetype.mask |= component_mask;
        } (components), ...);


        std::apply([&](auto&... component_rows) {
            (component_rows.push_back(components), ...);
        }, new_archetype.component_table);

        return std::move(new_archetype);
    }

private:
    static constexpr vecs::usize 
    s_registered_components_size { sizeof...(RegisteredComponents) };

    std::unordered_map<component_id_t, mask_t> _component_masks{};

    using archetypes_t = std::variant<archetype_t<RegisteredComponents...>>;
    std::unordered_map<component_id_t, std::vector<archetypes_t>> _archetypes{};

    std::array<std::type_info const*, 
        s_registered_components_size> _registered_components_type_info;

    vecs::usize _registered_components_count { 0 };
    vecs::reusable_id_t<vecs::entity_id_t> _entity_ids{};
};
    
} // namespace vecs

// (e.g.) Imagine we are registering components (in order): { Position, Velocity, Health }.
// At the start of the program, Position mask will be 0b1 (see _next_component_mask declaration).
// we add Position mask to archetype.id using "|=" operator.
// Then, we shift the 0b1 one time to the left. Next mask is now 0b10.
// This repeats for Velocity and Health.
// Resulting in archetype.id = 0b111.