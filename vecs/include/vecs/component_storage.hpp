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
#include <any>
#include <bitset>
#include <numeric>
#include <tuple>
#include <concepts>

// lib
#include "data_structures/resusable_id.hpp"
#include "data_structures/unique_tuple.hpp"
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
using component_id_t = decltype(std::declval<std::type_info>().hash_code());

// MAX_REGISTRABLE_COMPONENTS is defined as 64 to fit inside a 64-bit bitset.
static constexpr vecs::usize MAX_REGISTRABLE_COMPONENTS { 64 };
static constexpr vecs::usize MAX_ALIVE_ENTITIES { 1'000 }; // Arbitrarily set.

// Restrictions for a type to be considered as a Component:
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
    get_component(entity_id_t entity_id) const noexcept {
        // We can access component rows by their type thanks to the guarantee
        // of unique types inside the tuple (see unique_tuple.hpp).
        auto const& components = std::get<component_row_t<T>>(component_table);
        return components[entity_id];
    }

    std::tuple<Cs...>
    get_entity_components(entity_id_t entity_id) const noexcept {
        // Entity id is guaranteed to be >= 0.
        assert(entity_id < entity_count() && "Entity id out of bounds.");

        // Obtain a tuple of all the components in the entity id column.
        auto const component_column 
            = std::make_tuple(get_component<Cs>(entity_id)...);

        return component_column;
    }

    template <typename F>
    void 
    for_each(F&& function) {
        assert(!std::get<0>(component_table).empty() 
                && "Component table is empty.");

        vecs::usize const entity_count { std::get<0>(component_table).size() };

        for (vecs::usize entity_id{}; entity_id < entity_count; ++entity_id) {
            std::apply([&function, entity_id](auto&... component_row) {
                std::forward<F>(function)(component_row[entity_id]...);
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
        | Position  | (0.0, 0.0) | (-1.0, 2.0) | (3.2, 1.0) | <- Component row.
        | Velocity  | (0.0, 0.0) | (1.0, 1.0)  | (2.3, 0.0) |
        | Health    | 100        | 42          | 7          |
        +-----------+------------+-------------+------------+

        A column in the table represents an entity an it's components!
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
        
        _register_components(_registered_components_type_info);
    }

    template <Component... Cs>
    archetype_t<Cs...>&
    create_archetype(Cs&&... components) {
        return _get_or_create_archetype(std::forward<Cs>(components)...);
    }
    
    constexpr vecs::usize
    get_registered_component_count() const noexcept {
        return _registered_components_count;
    }

private:
    void
    _register_components(auto const& components_type_info) noexcept {
        for (auto const& component_info: components_type_info) {
            assert(_next_component_mask
                <= std::numeric_limits<decltype(_next_component_mask)>::max()
                && "Component registration limit reached.");

            component_id_t const component_id = component_info->hash_code();
            
            assert(!_component_masks.contains(component_id)
                && "Component already registered.");
                
            _component_masks[component_id] = _next_component_mask;  // (e.g.) Imagine we are registering components (in order): { Position, Velocity, Health }.
            _next_component_mask <<= 1;                             // At the start of the program, Position mask will be 0b1 (see _next_component_mask declaration).
                                                                    // we add Position mask to archetype.id using "|=" operator.
                                                                    // Then, we shift the 0b1 one time to the left. Next mask is now 0b10.
                                                                    // This repeats for Velocity and Health.
                                                                    // Resulting in archetype.id = 0b111.
            _registered_components_count += 1;
            
            // TODO - Remove this log.
            component_name_t const component_name = component_info->name();
            std::printf("%s -> %s\n", component_name, _component_masks[component_id].to_string().c_str());
        }
    }

    template <Component... Cs>
    void
    _register_archetype() noexcept {
        static_assert(sizeof...(Cs) > 0, 
            "Archetype must have at least 1 associated component.");
        assert(!_is_archetype_registered<Cs...>() 
            && "Archetype is already registered.");
        
        archetype_t<Cs...> new_archetype{};
        new_archetype.mask = (_get_component_mask<Cs>() | ...);

        _archetypes[new_archetype.mask] 
            = std::make_any<archetype_t<Cs...>>(new_archetype);
    }

    template <Component... Cs>
    void
    _add_entity(Cs&&... components) {
        archetype_t<Cs...>& archetype 
            = _get_or_create_archetype(std::forward<Cs>(components)...);

        assert(false && "Not implemented."); // TODO - Continue here (add entity to archetype).
    }

    template <Component C>
    [[nodiscard]] mask_t
    _get_component_mask() {
        component_id_t component_id = typeid(C).hash_code();
        assert(_component_masks.contains(component_id) 
            && "Component not registered.");

        return _component_masks[component_id];
    }

    template <Component... Cs>
    [[nodiscard]] bool
    _is_archetype_registered() {
        mask_t archetype_mask = (_get_component_mask<Cs>() | ...);
        return (_archetypes.contains(archetype_mask));
    }

    template <Component... Cs>
    [[nodiscard]] archetype_t<Cs...>&
    _get_or_create_archetype(Cs&&... components) {
        if (!_is_archetype_registered<Cs...>()) {
            _register_archetype<Cs...>();
        }
        
        mask_t const& archetype_mask { _get_archetype_mask<Cs...>() };
        return std::any_cast<archetype_t<Cs...>&>(_archetypes[archetype_mask]);
    }

    template <Component... Cs>
    [[nodiscard]] mask_t
    _get_archetype_mask() {
        assert(_is_archetype_registered<Cs...>() 
            && "Cannot get archetype mask, archetype is not registered.");
        return (_component_masks[typeid(Cs).hash_code()] | ...);
    }


private:
    static constexpr vecs::usize 
    s_registered_components_size { sizeof...(RegisteredComponents) };
    
    // Component Storage Data Layout.
    std::unordered_map<component_id_t, mask_t> _component_masks{};
    std::unordered_map<mask_t, std::any> _archetypes{};

    std::array<std::type_info const*, 
        s_registered_components_size> _registered_components_type_info;

    vecs::usize _registered_components_count { 0 };
    vecs::u64 _next_component_mask { 0b1 };
    vecs::reusable_id_t<vecs::entity_id_t> _entity_ids{};
};
    
} // namespace vecs

