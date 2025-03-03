#pragma once

// std
#include <tuple>
#include <vector>

// lib
#include "types.hpp"
#include "data_structures/slotmap.hpp"

namespace vecs {

template <Component... Cs>
struct archetype_t final {
public:
    static_assert(sizeof...(Cs) > 0, 
        "Archetype must have at least one component.");

    explicit archetype_t(vecs::mask_t mask) : _mask(mask) {}

    constexpr vecs::mask_t
    mask() const noexcept {
        return _mask;
    }

    constexpr vecs::usize
    entity_count() const noexcept {
        return std::get<0>(_component_table).size();
    }

    std::vector<component_key_t>
    add_entity(vecs::entity_id_t const entity_id, Cs&&... components) {
        // Add a column.
        std::vector<component_key_t> cmp_keys{};

        // C++17 fold expression for iterating each element in pack `components`.
        ([&](auto&& component) {
            using component_type_t = std::remove_reference_t<decltype(component)>;
            auto& component_row = 
                std::get<component_row_t<component_type_t>>(_component_table);
            vecs::component_key_t const key = component_row.push_back(component);

            cmp_keys.push_back(key);
        } (std::forward<Cs>(components)), ...);

        return cmp_keys;
    }

    template <Component C>
    vecs::component_key_t const
    store_component(C&& component) {
        auto& component_row = std::get<component_row_t<C>>(_component_table);
        auto const key = component_row.push_back(component);

        return key;
    }

    template <typename F>
    void 
    for_each(F&& function) {
        assert(!std::get<0>(_component_table).empty() 
                && "Component table is empty.");

        vecs::usize const entity_count { std::get<0>(_component_table).size() };

        for (vecs::usize entity_id{}; entity_id < entity_count; ++entity_id) {
            std::apply([&function, entity_id](auto&... component_row) {
                std::forward<F>(function)(component_row[entity_id]...);
            }, _component_table);
        }
    }

private:
    vecs::usize const _component_count { sizeof...(Cs) };

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
    vecs::mask_t _mask{};

    /*  Columns = Entity ids.
        Rows    = Components Types.

        Archetype visualization (Mask is 64-bit, truncated here for brevity):
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
    using component_row_t = vecs::slotmap_t<T, MAX_REGISTRABLE_COMPONENTS>;
    using component_table_t = std::tuple<component_row_t<Cs>...>;
    component_table_t _component_table{};
};
    
} // namespace vecs
