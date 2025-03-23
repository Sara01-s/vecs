#pragma once

// std
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <typeinfo>
#include <variant>
// Replace with dense_map in the future.
// Or use a robin hood hash map.
#include <any>
#include <bitset>
#include <numeric>
#include <tuple>
#include <unordered_map>
#include <vector>

// lib
#include "archetype.hpp"

namespace vecs {

template <vecs::usize MaxComponents = 64, vecs::usize MaxAliveEntities = 64>
struct component_storage_t final {
    using log_t = vecs::log_t; // TODO - Delete?

    component_storage_t() = default;
    ~component_storage_t() = default;

    // Since Component concept (see RegisteredComponents declaration) guarantees that a component is NOT polymorphic:
    // `typeid(component)` is resolved at *compile time*.
    // src: https://en.cppreference.com/w/cpp/language/typeid#:~:text=Notes,is%20resolved%20at%20compile%20time.
    template <Component... Cs>
    void
    register_components() noexcept {
        static_assert(
            sizeof...(Cs) > 0,
            "Component storage must have at least 1 registered component."
        );
        static_assert(
            sizeof...(Cs) <= MaxComponents,
            "Component storage capacity exceeded."
        );

        std::array<std::type_info const*, sizeof...(Cs)> components_type_info {
            &typeid(Cs)...
        };

        _register_components(components_type_info);
    }

    template <Component... Cs>
    std::vector<component_key_t>
    add_components(
        vecs::entity_id_t const entity_id,
        Cs&&... components
    ) noexcept {
        vecs::mask_t mask = (_component_masks[typeid(Cs).hash_code()] | ...);
        _entity_masks[entity_id] = mask;

        if (!_is_archetype_registered(mask)) {
            _register_archetype<Cs...>();
        }

        auto& archetype = _get_archetype<Cs...>();
        return archetype.add_entity(entity_id, std::forward<Cs>(components)...);
    }

    void
    remove_components(vecs::entity_id_t const entity_id) noexcept {
        vecs::mask_t const mask = _entity_masks[entity_id];
        _entity_masks[entity_id] = vecs::DEAD_ENTITY_ID;
    }

    template <Component... Cs>
    void
    for_each(auto&& system) {
        vecs::archetype_t<Cs...>& archetype = _get_archetype<Cs...>();
        archetype.for_each(std::forward<decltype(system)>(system));
    }

    [[nodiscard]] vecs::mask_t
    get_entity_mask(vecs::entity_id_t const entity_id) const noexcept {
        return _entity_masks[entity_id];
    }

private:
    template <Component... Cs>
    constexpr void
    _register_components(auto const& components_type_info) noexcept {
        for (auto const& component_info : components_type_info) {
            assert(
                _next_component_mask <=
                    std::numeric_limits<decltype(_next_component_mask)>::max(
                    ) &&
                "Component registration limit reached."
            );

            vecs::component_id_t const component_id =
                component_info->hash_code();

            assert(
                !_component_masks.contains(component_id) &&
                "Component already registered."
            );

            // (e.g.) Imagine we are registering components (in order): { Position, Velocity, Health }.
            _component_masks[component_id] = _next_component_mask;
            // At the start of the program, Position mask will be 0b1 (see _next_component_mask declaration).
            // we add Position mask to archetype.id using "|=" operator.
            // Then, we shift the 0b1 one time to the left. Next mask is now 0b10.
            // This repeats for Velocity and Health.
            // Resulting in archetype.id = 0b111.
            _next_component_mask <<= 1;

            // TODO - Remove this log.
            vecs::component_name_t const component_name =
                component_info->name();
            log_t::log(
                log_t::YELLOW,
                "Registered component: ",
                log_t::CLEAR,
                component_name,
                " with mask: ",
                log_t::LIGHT_MAGENTA,
                "0b",
                _component_masks[component_id].to_string()
            );
        }
    }

    template <Component... Cs>
    void
    _register_archetype() noexcept {
        static_assert(
            sizeof...(Cs) > 0,
            "Archetype must have at least 1 associated component."
        );

        assert(
            !_is_archetype_registered<Cs...>() &&
            "Archetype is already registered."
        );

        vecs::archetype_t<Cs...> archetype {(_get_component_mask<Cs>() | ...)};
        _archetypes[archetype.mask()] = std::any {std::move(archetype)};
    }

    template <Component C>
    [[nodiscard]] vecs::mask_t
    _get_component_mask() noexcept {
        vecs::component_id_t component_id = typeid(C).hash_code();
        assert(
            _component_masks.contains(component_id) &&
            "Component not registered."
        );

        return _component_masks[component_id];
    }

    template <Component... Cs>
    [[nodiscard]] bool
    _is_archetype_registered() noexcept {
        vecs::mask_t archetype_mask = (_get_component_mask<Cs>() | ...);
        return (_archetypes.contains(archetype_mask));
    }

    [[nodiscard]]
    constexpr bool
    _is_archetype_registered(vecs::mask_t archetype_mask) const noexcept {
        return (_archetypes.contains(archetype_mask));
    }

    template <Component... Cs>
    [[nodiscard]] vecs::archetype_t<Cs...>&
    _get_archetype() noexcept {
        vecs::mask_t const archetype_mask = _get_archetype_mask<Cs...>();

        assert(
            _is_archetype_registered(archetype_mask) &&
            "Archetype not registered."
        );

        return std::any_cast<vecs::archetype_t<Cs...>&>(
            _archetypes[archetype_mask]
        );
    }

    template <Component... Cs>
    [[nodiscard]] vecs::mask_t
    _get_archetype_mask() noexcept {
        assert(
            _is_archetype_registered<Cs...>() &&
            "Cannot get archetype mask, archetype is not registered."
        );
        return (_component_masks[typeid(Cs).hash_code()] | ...);
    }

private:
    vecs::u64 _next_component_mask {0b1};

    // Component Storage Data Layout.
    std::unordered_map<vecs::component_id_t, vecs::mask_t> _component_masks {};
    std::array<vecs::mask_t, MaxAliveEntities> _entity_masks {};
    std::unordered_map<vecs::mask_t, std::any> _archetypes {};
};

} // namespace vecs
